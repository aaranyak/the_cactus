/* Similar to search, but no depth limit and only evaluates capture moves */
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
#include "move_ordering.h"
#define INF INT_MAX

result_t quiescence(Bitboard *board, int alpha, int beta) {
    /* Evaluates moves only with no captures */
    // Declare for minmax
    int index; /* Useful for looping over moves */
    move_t move; /* Use this in loops */
    move_list_t pseudo_legal = {0,0}; /* Create a move list for pseudo-legal moves */
    move_list_t legal_moves = {0,0}; /* This list will only contain legal moves */
    // Generate legal moves
    generate_moves(board, &pseudo_legal); /* Generate pseudo legal moves */
    for (index = 0; index < pseudo_legal.count; index++) { /* Loop through the pseudo-legal moves to filter out illegal ones */
        move = pseudo_legal.moves[index]; /* Get the move */
        if (is_legal(board, move) && (move & MM_CAP)) { /* If this move is a legal capture */
            add_move_to_list(&legal_moves, move); /* Add it to the list of legal moves */
        }
    }

    if (legal_moves.count == 0) { /* There are no captures left */
        int evaluation = evaluate(board); /* Return the static evaluation */
        return (result_t){evaluation, 0}; /* Just return an evaluation */
    }
    
    order_moves(&legal_moves, board, 0, 0); /* Order moves to increase number of cutoffs during search */

    // There are captures left, continue search
    result_t result; /* Current result */
    move_t max_move = legal_moves.moves[0]; /* The move with the highest evaluation */
    int piece;
    int cap_piece;
    U64 castling, enpas, key; int ps_eval; /* Used for make/unmake */
    for (index = 0; index < legal_moves.count; index++) { /* Loop through all the legal moves */
        move = legal_moves.moves[index]; /* Current move */
        make_move(board, move, &enpas, &castling, &key, &ps_eval); /* Make the move on the board */
        result = quiescence(board, -beta, -alpha); /* Recursively call itself to search at an even higher depth */
        unmake_move(board, move, &enpas, &castling, &key, &ps_eval); /* Unmake the move on the board */

        // Alpha-beta pruning
        if (-result.evaluation >= beta) { /* Evaluation better than last best */
            /* Prune this branch, since the opponent will not consider this position */
            return (result_t){beta, move}; /* Need not search further */
        }

        if (alpha < -result.evaluation) {
            alpha = -result.evaluation; /* Update best eval */
            max_move = move; /* Update best move */
        }
    }

    return (result_t){alpha, max_move}; /* Return the result */
}
