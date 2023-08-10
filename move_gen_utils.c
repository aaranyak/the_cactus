/* move_gen_utils.c
 * Utilities for move generation.
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "lookup_tables.h"

U64 colour_mask(Bitboard *board, int side) {
    /* Returns a union of all the boards of a certian colour */
    if (side) { /* If the colour is white */
        return board->pieces[0] /* From 0 */
             | board->pieces[1]
             | board->pieces[2]
             | board->pieces[3]
             | board->pieces[4]
             | board->pieces[5]; /* To 5 (All white pieces) */
    } else { /* For black pieces */
        return board->pieces[6] /* From 6 */
             | board->pieces[7]
             | board->pieces[8]
             | board->pieces[9]
             | board->pieces[10]
             | board->pieces[11]; /* To 11 (All black pieces) */
    }
}

int get_captured_piece(Bitboard *board, U64 position, int side) {
    /* Gets the id of a captured piece */
    for (int piece_id = (side) ? 6 : 0; piece_id < (side) ? 12 : 6; piece_id++) { /* Loop through all the opponent pieces */
        if (board->pieces[piece_id] & position) /* if the position has an opponent piece */ return piece_id; /* Break out of the loop and return the id */
    }
    return 12;
}
