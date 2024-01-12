/* Similar to search, but no depth limit and only evaluates capture moves */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "init_magics.h"
#include "make_move.h"
#include "legality_test.h"
#include "generate_moves.h"
#include "lookup_tables.h"
#include "evaluation.h"
#include "search.h" /* result_t typedef */
#include "move_ordering.h"
#include "move_gen_utils.h"

#define INF INT_MAX
#define DELTA 200 /* Used for delta pruning */

int get_least_valuable_attacker(Bitboard *board, int side, int attacks) {
    /* Get the least valuable attacker of a square (For SEE) */
    if (side) {
        /* If for white */
        if (board->pieces[pawn_w] & attacks) return pawn_w; /* Return a pawn */
        if (board->pieces[knight_w] & attacks) return knight_w; /* Return a knight */
        if (board->pieces[bishop_w] & attacks) return bishop_w; /* Return a bishop */
        if (board->pieces[rook_w] & attacks) return rook_w; /* Return a rook */
        if (board->pieces[queen_w] & attacks) return queen_w; /* Return a queen */
    } else {
        /* Otherwise for black */
        if (board->pieces[pawn_b] & attacks) return pawn_b; /* Return a pawn */
        if (board->pieces[knight_b] & attacks) return knight_b; /* Return a knight */
        if (board->pieces[bishop_b] & attacks) return bishop_b; /* Return a bishop */
        if (board->pieces[rook_b] & attacks) return rook_b; /* Return a rook */
        if (board->pieces[queen_b] & attacks) return queen_b; /* Return a queen */
    }
    return -1;
}

int see(Bitboard *board, move_t move) {
    /* SEE - Static Exchange Evaluation.
     * This is a method of predicting the outcome of a series of captures on one square
     * This will be used to prune off quiescence search nodes which have SEE < 0, to not search bad captures.
     * This can be done recursively, but will actually be done using a loop
     * Reference from https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm
    */
    int exchange[32] = {0}; /* Everything gets captured */
    int depth = 0; /* Current depth */
    int piece = (move & MM_PIECE) >> MS_PIECE; /* Get the attacking piece */
    int captured = (move & MM_EAT) >> MS_EAT; /* Get the captured piece */
    U64 square = (move & MM_TO) >> MS_TO; /* Get the relevant attacked square to see on */
    U64 attacker = 1ULL << ((move & MM_FROM) >> MS_FROM); /* Get the from square */
    U64 occupied = colour_mask(board, 0) | colour_mask(board, 1); /* Get the occupied squares, to use as a temporary occupation map */
    U64 attacks = square_attacked_by(board, square, occupied); /* Get all the squares that attack this square */
    if (popcount(attacks) > 32) {
        render_board(board); /* Cursed */
        exit(1);
    }
    // Run the loop, simulating recursively going deeper.
    exchange[0] = materials[captured]; /* Calculate initial material exchange */
    do {
        depth++; /* Go one step deeper */
        exchange[depth] = materials[piece] - exchange[depth - 1]; /* Estimate the material exchange for this capture (assuming square is defended) */
        if (max(-exchange[depth - 1], exchange[depth]) < 0) break; /* Prune if material does not increase result */
        // Reset attackers
        occupied ^= attacker; /* Remove the attacker */
        attacks = square_attacked_by(board, square, occupied); /* Considering that x-rays might have shifted */
        // Get next attacker
        piece = get_least_valuable_attacker(board, board->side ^ (depth & 1), attacks); /* Get least valuable attacker */
        if (piece == -1) break;
        attacker = board->pieces[piece]; /* Get the piece bitboard */
        attacker &= attacks; /* Get if only an attacker */
        attacker &= -attacker; /* Separate the least significant bit */
    } while (attacker); /* Until there are no more attackers */
    while (--depth) /* Traverse the sequence backwards */
        exchange[depth - 1] = -max(-exchange[depth - 1], exchange[depth]); /* And retrieve the exchange */
    return exchange[0]; /* Return the final material exchange */
}

result_t quiescence(Bitboard *board, int alpha, int beta) {
    /* Evaluates moves only with no captures */
    // Declare for minmax
    int index; /* Useful for looping over moves */
    move_t move; /* Use this in loops */
    move_list_t pseudo_legal = {0,0}; /* Create a move list for pseudo-legal moves */
    move_list_t legal_moves = {0,0}; /* This list will only contain legal moves */

    // Evaluate Standing-Pat
    int evaluation = evaluate(board); /* Return evaluation */

    if (evaluation >= beta) /* alpha-beta pruning */
        return (result_t){beta, 0}; /* Prune this branch */
    if (evaluation > alpha) /* That Standing-PAT thing */
        alpha = evaluation; /* Set the alpha to the current evaluation */

    // Generate legal moves
    generate_moves(board, &pseudo_legal); /* Generate pseudo legal moves */
    for (index = 0; index < pseudo_legal.count; index++) { /* Loop through the pseudo-legal moves to filter out illegal ones */
        move = pseudo_legal.moves[index]; /* Get the move */
        if (is_legal(board, move) && (move & MM_CAP)) { /* If this move is a legal capture */
            add_move_to_list(&legal_moves, move); /* Add it to the list of legal moves */
        }
    }

    if (legal_moves.count == 0) { /* There are no captures left */
        return (result_t){evaluation, 0}; /* Just return an evaluation */
    }
    
    order_moves(&legal_moves, board, 0, 0, 1); /* Order moves to increase number of cutoffs during search */

    // There are captures left, continue search
    result_t result; /* Current result */
    move_t max_move = legal_moves.moves[0]; /* The move with the highest evaluation */
    int piece;
    int cap_piece;
    U64 castling, enpas, key; int ps_eval; /* Used for make/unmake */
    for (index = 0; index < legal_moves.count; index++) { /* Loop through all the legal moves */
        move = legal_moves.moves[index]; /* Current move */

        // Delta pruning
        int cap_piece = (move & MM_EAT) >> MS_EAT;
        if ((evaluation + materials[cap_piece] + DELTA) < alpha) /* If the evaluation + the captured piece material + some margin cannot raise the alpha, prune this branch */
            continue;
        
        // Static Exchange Evaluation Pruning
        if (evaluation + see(board, move) < alpha) /* If the static exchange evaluation is less than 0 */
            continue; /* Just prune this branch */

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
