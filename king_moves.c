/* king moves.c
 * Contains functions t generate all king moves and add them to moves list.
 *  -> Move generation by lookup
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "lookup_tables.h"
#include "move_gen_utils.h"

void add_king_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int king_index, U64 enemy_mask);

void generate_king_moves(move_list_t *move_list, Bitboard *board) {
    /* Generates all the king moves and adds them to move list */
    // Declare
    int side = board->side; /* Side to move */
    U64 own_mask = colour_mask(board, side); /* Piece mask of own side */
    U64 enemy_mask = colour_mask(board, !side); /* Piece mask of enemy pieces */
    U64 king = (side) ? board->pieces[king_w] : board->pieces[king_b]; /* King bitboard */
    U64 move_set; /* Set of king moves */
    int king_index; /* Index of king */
    // Generate king move set
    king_index = bitscan(king); /* Get the index of the king */
    move_set = king_attacks[king_index]; /* Get the king attacks from this position */
    move_set &= ~own_mask; /* Remove blocked squared */
    
    add_king_moves(move_set, board, move_list, king_index, enemy_mask); /* Add all the king moves to the list */
} /* A surpisingly simple function, since there is only one king for each side, and he cannot ever be captured */

void add_king_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int king_index, U64 enemy_mask) {
    /* Add all king moves to move list */
    int side = board->side; /* Makes code writing easier */
    // Declare for loop
    U64 position;
    U64 cap_flag; /* Check if move is a capture */
    int pos_index;
    int cap_piece; /* Captured piece (if any) */
    move_t move; /* Current move */

    while (move_set) { /* Loop through all the moves in the set */
        position = move_set & -move_set; /* Get next move */
        pos_index = bitscan(position); /* Get index of to move */
        cap_flag = position & enemy_mask; /* Check if this is a capture */
        if (cap_flag) cap_piece = get_captured_piece(board, position, side); /* Get captured piece id (if there is one) */
        move = set_move(king_index /* from */, pos_index /* to */, (side) ? king_w : king_b /* Piece */, (cap_flag) ? cap_piece : 0 /* Captured Piece */); /* Set the move */
        // Set flags
        if (cap_flag) move |= MM_CAP; /* If this is a capture move, set the capture flag */
        // Next
        add_move_to_list(move_list, move); /* Add move to list (obviously) */
        move_set ^= position; /* Remove move from set (Reset LSB) */
    }
}
