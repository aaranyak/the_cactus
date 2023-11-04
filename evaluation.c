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
    
    // Add endgame eval
    int king_dist_w, king_dist_b; /* King distance from corner evaluation */
    king_dist_w = center_manhattan_distance[bitscan(board->pieces[king_w])] * -KING_CORNER_WEIGHT;
    king_dist_b = center_manhattan_distance[bitscan(board->pieces[king_b])] * -KING_CORNER_WEIGHT;
    // Multiply by endgame-weights
    king_dist_w *= endgame_w;
    king_dist_b *= endgame_b;
    
    evaluation += side ? king_dist_w - king_dist_b : king_dist_b - king_dist_w; /* Add it to the evaluation */


    // Add piece square evaluation
    int pst_eval; /* Piece-square evaluation */
    pst_eval = board->piece_square_eval;
    if (!side) pst_eval = (pst_eval * opening_weight(board)) / -OPENING_END; /* Flip the piece_square eval if black is playing */
    else pst_eval = (pst_eval * opening_weight(board)) / OPENING_END;
    evaluation += pst_eval;

    return evaluation;
}
