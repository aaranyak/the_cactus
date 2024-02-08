/* This file will hold the search function
 * Search function:
    -> Minmax with alpha-beta pruning
    -> Quiscience search
    -> Move ordering
    -> etc.
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "move_gen_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "init_magics.h"
#include "make_move.h"
#include "legality_test.h"
#include "generate_moves.h"
#include "evaluation.h"
#include "search.h" /* result_t typedef */
#include "quiescence.h"
#include "move_ordering.h"
#include "zobrist_hash.h"
#include "tp_table.h"
#include "killer_moves.h"

#define INF INT_MAX
#define R 2 /* Null move reduction */
#define MAX_EXTENSIONS 10

int thread_count = 8; /* This should not exist */

result_t search(Bitboard *board, int depth, int alpha, int beta, int *interrupt_search, int max_time, move_t force_move, int ply, int extensions, int randomize_search) {
    /* Generate moves, recursively generate moves from resulting positions until
     * maximum depth is reached, and then evaluate the position, use minmax
     * algorithm to find best evaluation and move.
    */
    // Check time and search interrupt for iterative deepening
    if ((int)time(NULL) >= max_time) /* If the search time has been exceeded */
        *interrupt_search = 1; /* Stop searching */

    // Search for entry in tp_table
    entry_t entry = get_entry(board->key); /* Try getting the entry from the tp-table */
    if (!invalid_entry(entry) && entry.depth >= depth && entry.node_type == node_pv) { /* If the entry is there, and the depth of the entry is greater than or equal to the current depth, and this is a pv node */
        // Use the evaluation from the table
        hash_move_used++;
        return (result_t){entry.eval, entry.best_move}; /* Return the results from the table entry */
    }

    if (depth == 0) { /* Reached end of search */
        /* Use Quiscience search over here */
        return quiescence(board, alpha, beta); 
    } else { /* Still not reached end of search */
        // Declare for minmax
        int index; /* Useful for looping over moves */
        move_t move; /* Use this in loops */
        move_list_t pseudo_legal = {0,0}; /* Create a move list for pseudo-legal moves */
        move_list_t legal_moves = {0,0}; /* This list will only contain legal moves */
        
        // Generate legal moves
        generate_moves(board, &pseudo_legal); /* Generate pseudo legal moves */
        for (index = 0; index < pseudo_legal.count; index++) { /* Loop through the pseudo-legal moves to filter out illegal ones */
            move = pseudo_legal.moves[index]; /* Get the move */
            if (is_legal(board, move)) { /* If this move is legal */
                add_move_to_list(&legal_moves, move); /* Add it to the list of legal moves */
            }
        }
        
        // Check if this is checkmate
        if (legal_moves.count == 0) { /* Checkmate/Stalemate */
            if (is_check(board, board->side)) { /* If this is a check */
                return (result_t){-INF, 0}; /* Return Checkmate */
            } else { /* Stalemate */
                return (result_t){0, 0}; /* Return stalemate */
            }
        }
        
    //    // Check if this is a draw
    //    U64 piece_boards = colour_mask(board, 0) | colour_mask(board, 1); /* Get the total board */
    //    if (popcount(piece_boards) < 3) /* If only two kings are left */
    //        return (result_t){0, 0}; /* Return a draw (only two kings left) */
    //    // Three fold repetition
       if (get_repetitions(board) >= 3) /* Check if the position repeats more than 2 times */
           return (result_t){0, 0}; /* Return a draw. */
            
        
        // Order the moves
        if (randomize_search) { /* Randomize at root node (Lazy SMP) */
            /* Randomize the move order */
            randomize_moves(&legal_moves); /* Reorder the moves randomly */
        } else if (force_move) { /* If a forced move is given */
            /* Order moves with the forced move at first */
            order_moves(&legal_moves, board, 1, force_move, ply); /* Order Moves */
        } else if (invalid_entry(entry)) { /* If there is no hash move */
            /* Use the normal move ordering technique */
            order_moves(&legal_moves, board, 0, 0, ply); /* Order moves to increase number of cutoffs during search */
        } else { /* If there is a hash move available */
            order_moves(&legal_moves, board, 1, entry.best_move, ply); /* Again, Hash move - Best move ! */
        }
        // This is not a checkmate, continue search
        node_t node_type = node_all; /* At first assume all-node */
        result_t result; /* Current result */
        move_t max_move = (force_move) ? 0 : legal_moves.moves[0]; /* The move with the highest evaluation */
        U64 castling, enpas, key; int ps_eval; /* Used for make/unmake */
        
        // Extension/Reduction Data
        int reduction;
        int in_check = is_check(board, board->side); /* If in check, don't do LMR */
        int extension;

        // Null Move Pruning (Basically check if null move causes beta cutoff.)
        int null_move_reduction = depth - R - 1; /* First value of R */
        if (!in_check && ply > 1 && cutoff(null_move_reduction)) { /* Or in illegal position */
            board->side ^= 1; /* Switch sides */
            board->key ^= side_hash; /* Hash */
            board->moves++; /* Moves */
            // Do the actual search.
            result = search(board, null_move_reduction, -beta, -alpha, interrupt_search, max_time, 0, ply + 1, extensions, 0); /* Recursively call itself to search at an even higher depth */
            board->side ^= 1; /* Toggle side */
            board->key ^= side_hash; /* Yep... */
            board->moves--; /* Minus Minus */
            
            if (-result.evaluation >= beta) {
                /* Check for beta cuttof */
                return (result_t){beta, 0}; /* Because there is no move */
            }
        }

        for (index = 0; index < legal_moves.count; index++) { /* Loop through all the legal moves */
            move = legal_moves.moves[index]; /* Current move */
            make_move(board, move, &enpas, &castling, &key, &ps_eval); /* Make the move on the board */
            

            // Search Extensions
            /* Currently, only a check extension is implemented, extending the search by one ply if in check */
            int gives_check = is_check(board, board->side); /* If this move gives a check */
            extension = 0;
            if (gives_check && extensions < MAX_EXTENSIONS) extension = 1;


            // Late Move Reductions
            /* Late Move Reductions:
             *  -> This is an optimisation, assuming that the move ordering scheme works
             *  -> We search the later moves to a lower depth, therefore saving search time.
             *  -> In the case that the move does increase alpha,
             *  -> We do another search to the regular depth
            */
            // Calculate Reduction Amount.
            reduction = 0; /* In the first 5 moves, don't reduce */
            if (!((move & MM_CAP || move & MM_EPC || MM_PRO) || /* If not a tactical move */ in_check /* If in check, don't risk reductions */ || extension /* If the search has an extension, dont use LMR */) && depth > 3 /* Don't risk LMR in low depths */) { /* If it's ok to use LMR */
                if (index > 4) reduction = 1; /* After 5 moves, reduce by 1 */
                if (index > 15) reduction = 2; /* For last few moves, reduce by two counts */
            }

            result = search(board, cutoff(depth - 1 - reduction + extension), -beta, -alpha, interrupt_search, max_time, 0, ply + 1, extensions + extension, 0); /* Recursively call itself to search at an even higher depth */

            if (reduction && -result.evaluation > alpha) /* If a reduced move increases alpha */
                /* Then do another search to the full depth */
                result = search(board, depth - 1, -beta, -alpha, interrupt_search, max_time, 0, ply + 1, extensions, 0); /* Recursively call itself to search at an even higher depth */

            unmake_move(board, move, &enpas, &castling, &key, &ps_eval); /* Unmake the move on the board */
            
            // Check for search interrupt
            if (*interrupt_search) /* If the search has been interrupted */
                return (result_t){alpha,max_move}; /* Get out */
            
            // Alpha-beta pruning
            if (-result.evaluation >= beta) { /* Evaluation better than last best */
                /* Prune this branch, since the opponent will not consider this position */
                node_type = node_cut; /* Set it to a cut node, since this branch will be pruned */
                add_entry(board->key, beta, depth, board->moves, move, node_type); /* Add the entry to the transposition table */
                
                // Add killer move (if killer)
                if (!(move & MM_PRO || move & MM_CAP || move & MM_EPC)) { /* If this is not a capture or promotion move */
                    // Add it to the killer moves list
                    add_killer(move, ply); /* Add the move to the killer moves list */
                }

                return (result_t){beta, move}; /* Need not search further */
            }

            if (alpha < -result.evaluation) {
                node_type = node_pv; /* Alpha has been updated, set it to a pv-node */
                alpha = -result.evaluation; /* Update best eval */
                max_move = move; /* Update best move */
            }
        }
        
        // Update result in TP Table
        add_entry(board->key, alpha, depth, board->moves, max_move, node_type); /* Add the entry to the transposition table */

        return (result_t){alpha, max_move}; /* Return the result */
    }
}


struct helper_thread_data {
    /* To pass data down to other threads */
    Bitboard *board;
    int depth;
    int alpha;
    int beta;
    int *interrupt_search;
    int max_time;
};
void *start_helper_thread(void *uncasted_data) {
    /* Starts the helper search */
    struct helper_thread_data *data = (struct helper_thread_data*)uncasted_data; /* Cast the data */
    Bitboard board; /* This is a copy of the actual board, so as not to corrupt it */
    memcpy(&board, data->board, sizeof(Bitboard)); /* Copy the regular board into the copy */
    search(&board, data->depth, data->alpha, data->beta, data->interrupt_search, data->max_time, 0, 1, 0, 1); /* Helper thread with randomized move order */
    free(data); /* Because we don't need it anymore */
}

result_t start_parallel_search(Bitboard *board, int depth, int alpha, int beta, int *interrupt_search, int max_time, move_t force_move, int num_threads) {
    /* Starts a parallel search using multiple threads */
    pthread_t helper_threads[MAX_THREADS] = {0}; /* Maximum number of threads */
    struct helper_thread_data *data; /* Data for the threads to use */
    pthread_setconcurrency(3);
    // Create all the threads
    num_threads = min(num_threads, MAX_THREADS); /* Limit the number of threads */
    for (int i = 0; i < num_threads; i++) { /* Start creating helper threads */
        // Loop through each of them.
        data = (struct helper_thread_data*)malloc(sizeof(struct helper_thread_data)); /* Allocate memory for the data to pass to the thread */
        data->board = board; data->depth = depth; data->alpha = alpha; data->beta = beta; data->interrupt_search = interrupt_search; data->max_time = max_time; /* Put data in passing struct */
        pthread_create(&helper_threads[i] /* Put it here */, NULL, start_helper_thread, data); /* Start the thread */
    }

    result_t result = search(board, depth, alpha, beta, interrupt_search, (depth >= 4) ? max_time : INF, (depth > 1) ? force_move : 0, 1, 0, 0); /* Run the main thread with a force-move */

    // Join all the threads
    void *who_cares; /* We don't need a return value */
    for (int i = 0; i < num_threads; i++) { /* Loop through all the threads */
        pthread_join(helper_threads[i], &who_cares); /* Exit the thread */
    }

    return result; /* Don't be useless */
}

id_result_t iterative_deepening(Bitboard *board, int search_time) {
    /* Searches the board using iterative deepening */
    int depth = 0; /* Current depth */
    int max_time; /* Maximum time */
    int interrupt_search = 0; /* Whether to interrupt the search */
    id_result_t result; /* The final iterative deepening result */
    result_t current_result; /* The Current Result */
    
    max_time = (int)time(NULL); /* Get the current time */
    max_time += search_time; /* Add the time taken to search */
    hash_move_used = 0;
    while (!interrupt_search) { /* Until the search has not been interrupted */
        // Set the previous result
        // Do the search
        depth++; /* Increase the depth */
        clear_killers(); /* Clear killer moves */
        clear_history(); /* Clear history heuristic */
        current_result = start_parallel_search(board, depth, -INF, INF, &interrupt_search, max_time, result.move, thread_count); /* Search at the current depth */
        if (current_result.move == 0) break; /* If the search is interrupted before anything happens, get out. */
        result.evaluation = current_result.evaluation;
        result.move = current_result.move;
        result.depth = depth;
    }
    printf("hash move used - %d\n", hash_move_used);
    return result;
}
