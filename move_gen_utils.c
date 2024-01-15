/* move_gen_utils.c
 * Utilities for move generation.
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "lookup_tables.h"
#include "generate_moves.h"
#include "pawn_moves.h"
#include "king_moves.h"
#include "knight_moves.h"
#include "rook_moves.h"
#include "bishop_moves.h"
#include "queen_moves.h"
#include "castling_moves.h"

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

U64 attack_mask(Bitboard *board, int side) {
    /* Returns a union of all the boards of a certian colour */
    if (side) { /* If the colour is white */
        return board->attack_tables[0] /* From 0 */
             | board->attack_tables[1]
             | board->attack_tables[2]
             | board->attack_tables[3]
             | board->attack_tables[4]
             | board->attack_tables[5]; /* To 5 (All white attack_tables) */
    } else { /* For black attack_tables */
        return board->attack_tables[6] /* From 6 */
             | board->attack_tables[7]
             | board->attack_tables[8]
             | board->attack_tables[9]
             | board->attack_tables[10]
             | board->attack_tables[11]; /* To 11 (All black attack_tables) */
    }
}

int get_captured_piece(Bitboard *board, U64 position, int side) {
    /* Gets the id of a captured piece */
    for (int piece_id = (side) ? 6 : 0; piece_id < (side) ? 12 : 6; piece_id++) { /* Loop through all the opponent pieces */
        if (board->pieces[piece_id] & position) /* if the position has an opponent piece */ return piece_id; /* Break out of the loop and return the id */
    }
    return 12;
}

U64 square_attacked_by(Bitboard *board, int square, U64 occupied) {
    /* Square attacked by:
     * Calculates all attacking/defending pieces of specific squares.
    */
    U64 attacked_by = 0;
    // Jumping Piece Attacks
    attacked_by |= (pawn_attacks_b[square] & board->pieces[pawn_w]) /* White pawn attacks */ | (pawn_attacks_w[square] & board->pieces[pawn_b]) /* Black pawn attacks */;
    attacked_by |= knight_attacks[square] & (board->pieces[knight_w] | board->pieces[knight_b]); /* Knight attacks */
    attacked_by |= king_attacks[square] & (board->pieces[king_w] | board->pieces[king_b]); /* King attacks */
    // Sliding piece attacks.
	attacked_by |= magic_rook_moves(square, 0, occupied) & (board->pieces[rook_w] | board->pieces[rook_b]); /* Get rook attacks */
	attacked_by |= magic_bishop_moves(square, 0, occupied) & (board->pieces[bishop_w] | board->pieces[bishop_b]); /* Get bishop attacks */
	attacked_by |=  magic_queen_moves(square, 0, occupied) & (board->pieces[queen_w] | board->pieces[queen_b]); /* Get queen attacks */

    attacked_by &= occupied; /* Remove captured pieces */
    return attacked_by;
}
