/* evaluation.c
 * File holds the static (Handcrafted) evaluation function for a board position
*/
#include <stdio.h>
#include <stdlib.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "move_gen_utils.h"
#include "lookup_tables.h"
#include "init_magics.h"
#include "make_move.h"
#include "legality_test.h"
#include "generate_moves.h"

#define OPENING_END 25
#define KING_CORNER_WEIGHT 10
#define ENDGAME_MAXMAT 1600.0f
#define KING_SAFETY_WEIGHT -5
#define pawns_material(pawnsbb) (popcount((pawnsbb)) * 100)

int count_material(Bitboard *board, int side) {
    /* Counts the material on the board */
    int material = 0;
    int piece;
    if (side) for (piece = 0; piece < 6; piece++) material += popcount(board->pieces[piece]) * materials[piece]; /* Add the material*/
    else for (piece = 6; piece < 12; piece++) material += popcount(board->pieces[piece]) * materials[piece]; /* Add the material*/
    return material;
}

int opening_weight(Bitboard *board) {
    /* A weight to taper of opening evaluations */
    int weight = OPENING_END - board->moves;
    weight = cutoff(weight); /* Clamp at 0 */
    return weight;
}

int king_dist_eval(Bitboard *board, float endgame_w, float endgame_b) {
    /* King distance from center evaluation bonus */
    int king_dist_w, king_dist_b; /* King distance from corner evaluation */
    king_dist_w = center_manhattan_distance[bitscan(board->pieces[king_w])] * -KING_CORNER_WEIGHT;
    king_dist_b = center_manhattan_distance[bitscan(board->pieces[king_b])] * -KING_CORNER_WEIGHT;
    // Multiply by endgame-weights
    king_dist_w *= endgame_w;
    king_dist_b *= endgame_b;
    return board->side ? king_dist_w - king_dist_b : king_dist_b - king_dist_w; /* Add it to the evaluation */
}

int king_safety_eval(Bitboard *board, U64 occupied) {
    /* King safety eval - If king area is being attacked, add negative bias */
    int king_safety_w, king_safety_b; /* King squares being attacked */
    
    // Get king attacks
    U64 king_attacked_w = king_attacks[bitscan(board->pieces[king_w])]; /* Get the king attacks for white */
    U64 king_attacked_b = king_attacks[bitscan(board->pieces[king_b])]; /* Get the king attacks for black */

    // Find which one of those squares are attacked
    king_attacked_w &= attack_mask(board, 0); /* Only the attacked squares */
    king_attacked_b &= attack_mask(board, 1); /* Only the attacked squares */

    // Remove blocked squares
    king_attacked_w &= ~occupied;
    king_attacked_b &= ~occupied;

    king_safety_w = popcount(king_attacked_w) * KING_SAFETY_WEIGHT;
    king_safety_b = popcount(king_attacked_b) * KING_SAFETY_WEIGHT;

    return board->side ? king_safety_w - king_safety_b : king_safety_b - king_safety_w;

}

int pawn_endgame_eval(Bitboard *board, float endgame_w, float endgame_b) {
    /* Adds extra weightage for pawns that are higher up on the board */
    int w_pawn_eval = 0; /* Pawn eval for white */
    int b_pawn_eval = 0; /* Ditto for black */
    U64 pawn; /* Current pawn position */
    U64 pawns; /* Pawns bitboard */
    int pawn_index; /* Pawn position index */
    
    // Get white pawn bonus
    pawns = board->pieces[pawn_w]; /* Get the white pawns bitboard */
    while (pawns) { /* Loop through each of the pawns */
        pawn = pawns & -pawns; /* Get next pawn (Isolate LSB) */
        pawn_index = bitscan(pawn); /* Get the position index of this pawn */
        w_pawn_eval += endgame_pawn_eval[pawn_index]; /* Add the bonus */
        pawns ^= pawn; /* Reset LSB */
    }

    // Get black pawn bonus
    pawns = board->pieces[pawn_b]; /* Get the black pawns bitboard */
    while (pawns) { /* Loop through each of the pawns */
        pawn = pawns & -pawns; /* Get next pawn (Isolate LSB) */
        pawn_index = bitscan(pawn); /* Get the position index of this pawn */
        if (63 - pawn_index < 0) printf("Seriously...???\n"); /* Is this causing the bus error (This is soo cursed) */
        b_pawn_eval += endgame_pawn_eval[63 - pawn_index]; /* Add the bonus */
        pawns ^= pawn; /* Reset LSB */
    }
    
    w_pawn_eval *= endgame_w; /* Multiply by endgame weight */
    b_pawn_eval *= endgame_b; /* Multiply by endgame weight */

    return board->side ? w_pawn_eval - b_pawn_eval : b_pawn_eval - w_pawn_eval; /* Return the evaluation for the correct side */

}

int evaluate(Bitboard *board) {
    /* Statically evaluate the board */
    
    int evaluation = 0;
    int side = board->side; /* Makes writing code faster */ 
    // Calculate material
    int mat_w, mat_b;
    mat_w = count_material(board, 1); /* White Material */
    mat_b = count_material(board, 0); /* Black material */
    evaluation += side ? mat_w - mat_b : mat_b - mat_w; /* Add material depending on side */
    
    // Endgame-Weight
    float endgame_w, endgame_b; /* Endgame Weights */
    endgame_w = mat_w - pawns_material(board->pieces[pawn_w]); endgame_b = mat_b - pawns_material(board->pieces[pawn_b]); /* Get the without pawn materials */
    endgame_w = cutoff(ENDGAME_MAXMAT - endgame_w) / ENDGAME_MAXMAT; /* Endgame weight for white */
    endgame_b = cutoff(ENDGAME_MAXMAT - endgame_b) / ENDGAME_MAXMAT; /* Endgame weight for black */
    
    // Colour Mask
    U64 white_mask = colour_mask(board, 1); /* White pieces mask */
    U64 black_mask = colour_mask(board, 0); /* White pieces mask */

    // King distance from center evaluation.
    evaluation += king_dist_eval(board, endgame_w, endgame_b); /* Add that to the eval */

    // King safety bonus, decrease eval if king area is being attacked.
    evaluation += king_safety_eval(board, white_mask | black_mask); /* Add it to the evaluation */    

    // Endgame Pawn Eval
    evaluation += king_dist_eval(board, endgame_w, endgame_b); /* Add the pawn bonus to the eval weighted by endgame value */

    // Add piece square evaluation
    int pst_eval; /* Piece-square evaluation */
    pst_eval = board->piece_square_eval;
    if (!side) pst_eval = (pst_eval * opening_weight(board)) / -OPENING_END; /* Flip the piece_square eval if black is playing */
    else pst_eval = (pst_eval * opening_weight(board)) / OPENING_END;
    evaluation += pst_eval;

    return evaluation;
}
