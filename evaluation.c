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

int count_material(Bitboard *board, int side) {
    /* Counts the material on the board */
    int material = 0;
    int piece;
    for (piece = 0; piece < 6; piece++) material -= popcount(board->pieces[piece]) * materials[piece]; /* Add the material*/
    for (piece = 6; piece < 12; piece++) material += popcount(board->pieces[piece]) * materials[piece]; /* Add the material*/
    if (side) material = -material;
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
    
    int evaluation, material, pst_eval;

    // Count material
    material = count_material(board, board->side); /* Side's material - antisides material */
    // Add piece square evaluation
    pst_eval = board->piece_square_eval;
    if (!board->side) pst_eval = (pst_eval * opening_weight(board)) / -OPENING_END; /* Flip the piece_square eval if black is playing */
    else pst_eval = (pst_eval * opening_weight(board)) / OPENING_END;
    evaluation = pst_eval + material;

    return evaluation;
}
