/* castling_moves.c
 * Generate all possible castling moves from a position.
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "move_gen_utils.h"
#include "lookup_tables.h"
#include "init_magics.h"

// Castling checking bitmasks
#define W_KINGSIDE 0x0000000000000060
#define W_QUEENSIDE 0x000000000000000E
#define B_KINGSIDE 0x6000000000000000
#define B_QUEENSIDE 0x0E00000000000000


void generate_castling_moves(move_list_t *move_list, Bitboard *board) {
    /* Generate all possible castling moves from a position */
    int side = board->side; /* Convenience */
    U64 own_mask = colour_mask(board, side); /* Get own team mask */
    U64 enemy_mask = colour_mask(board, !side); /* Get enemy mask */
    U64 all_mask = own_mask | enemy_mask; /* Mask of both sides */
    // King's-side castling
    if (
            (board->castling_rights & ((side) ? WK_CASTLE : BK_CASTLE)) && /* Check if king/king-rook hasn't moved */
            (board->pieces[(side) ? rook_w : rook_b] & ((side) ? 128 : 0x8000000000000000)) && /* Check if king rook still exists */
            ((all_mask & ((side) ? W_KINGSIDE : B_KINGSIDE)) == 0) /* Check if no pieces are blocking */
       ) {
        /* Add move to move list */
        move_t move = MM_CAS;
        add_move_to_list(move_list, move);
    }
    // Queen's-side castling
    if (
            (board->castling_rights & ((side) ? WQ_CASTLE : BQ_CASTLE)) && /* Check if queen/queen-rook hasn't moved */
            (board->pieces[(side) ? rook_w : rook_b] & ((side) ? 1 : 0x0100000000000000)) && /* Check if queen rook still exists */
            ((all_mask & ((side) ? W_QUEENSIDE : B_QUEENSIDE)) == 0) /* Check if no pieces are blocking */
       ) {
        /* Add move to move list */
        move_t move = MM_CAS | MM_CSD;
        add_move_to_list(move_list, move);
    }
} 
