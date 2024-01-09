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
#include "bitboards.h"
#include "bitboard_utils.h"
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


result_t search(Bitboard *board, int depth, int alpha, int beta, int *interrupt_search, int max_time, move_t force_move) {
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
        
        
        // Order the moves
        if (force_move) { /* If a forced move is given */
            /* Order moves with the forced move at first */
            order_moves(&legal_moves, board, 1, force_move, depth); /* Order Moves */
        } else if (invalid_entry(entry)) { /* If there is no hash move */
            /* Use the normal move ordering technique */
            order_moves(&legal_moves, board, 0, 0, depth); /* Order moves to increase number of cutoffs during search */
        } else { /* If there is a hash move available */
            order_moves(&legal_moves, board, 1, entry.best_move, depth); /* Again, Hash move - Best move ! */
        }

        // This is not a checkmate, continue search
        node_t node_type = node_all; /* At first assume all-node */
        result_t result; /* Current result */
        move_t max_move = legal_moves.moves[0]; /* The move with the highest evaluation */
        U64 castling, enpas, key; int ps_eval; /* Used for make/unmake */
        for (index = 0; index < legal_moves.count; index++) { /* Loop through all the legal moves */
            move = legal_moves.moves[index]; /* Current move */
            make_move(board, move, &enpas, &castling, &key, &ps_eval); /* Make the move on the board */
            result = search(board, depth - 1, -beta, -alpha, interrupt_search, max_time, 0); /* Recursively call itself to search at an even higher depth */
            unmake_move(board, move, &enpas, &castling, &key, &ps_eval); /* Unmake the move on the board */
            
            // Alpha-beta pruning
            if (-result.evaluation >= beta) { /* Evaluation better than last best */
                /* Prune this branch, since the opponent will not consider this position */
                node_type = node_cut; /* Set it to a cut node, since this branch will be pruned */
                add_entry(board->key, beta, depth, board->moves, move, node_type); /* Add the entry to the transposition table */
                
                // Add killer move (if killer)
                if (!(move & MM_PRO || move & MM_CAP || move & MM_EPC)) { /* If this is not a capture or promotion move */
                    // Add it to the killer moves list
                    add_killer(move, depth); /* Add the move to the killer moves list */
                }

                // Check for search interrupt
                if (*interrupt_search) /* If the search has been interrupted */
                    return (result_t){beta,move}; /* Get out */
                
                return (result_t){beta, move}; /* Need not search further */
            }

            if (alpha < -result.evaluation) {
                node_type = node_pv; /* Alpha has been updated, set it to a pv-node */
                alpha = -result.evaluation; /* Update best eval */
                max_move = move; /* Update best move */
            }
            
            // Check for search interrupt
            if (*interrupt_search) /* If the search has been interrupted */
                return (result_t){alpha,max_move}; /* Get out */
        }
        

        // Update result in TP Table
        add_entry(board->key, alpha, depth, board->moves, max_move, node_type); /* Add the entry to the transposition table */

        return (result_t){alpha, max_move}; /* Return the result */
    }
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

    while (!interrupt_search) { /* Until the search has not been interrupted */
        // Set the previous result
        // Do the search
        depth++; /* Increase the depth */
        clear_killers(); /* Clear killer moves */
        current_result = search(board, depth, -INF, INF, &interrupt_search,(depth >= 4) ? max_time : INF, (depth > 1) ? result.move : 0); /* Search at the current depth */
        
        result.evaluation = current_result.evaluation;
        result.move = current_result.move;
        result.depth = depth;
    }

    return result;
}
