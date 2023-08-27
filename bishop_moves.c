/* bishop_moves.c
 * Generate all possible bishop moves.
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

U64 magic_bishop_moves(int square, U64 own, U64 enemy) {
    /* Get the bishop move set from magic attack table */
    U64 occupancies = (own | enemy) & bishop_masks[square]; /* Get occupancy bits */
    int index = (occupancies * bishop_magics[square]) >> bishop_shifts[square]; /* Index of the magic attacks on the board */
    U64 attacks = bishop_attacks[square][index]; /* Lookup the attacks from the pre-initialized table */
    attacks &= ~own; /* Remove squares blocked by own pieces from the move set */
    return attacks;
}
void add_bishop_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int from, U64 enemy_mask); /* Forward decleration */

void generate_bishop_moves(move_list_t *move_list, Bitboard *board) {
    /* Generates all possible moves by moving bishops, and adds them to the move list */
    // Masks
    int side = board->side; /* Side to move */
    U64 own_mask = colour_mask(board, side); /* Piece mask of own side */
    U64 enemy_mask = colour_mask(board, !side); /* Piece mask of enemy pieces */
    // Declare for loop
    U64 bishops = (side) ? board->pieces[bishop_w] : board->pieces[bishop_b]; /* Bishops bitboard */
    U64 position = 0; /* Bishop position (current bit) */
    U64 move_set = 0; /* Set of bishop moves */
    int bishop_index; /* Index of current bishop */
    
    while (bishops) { /* Loop through every bishop position */
        position = bishops & -bishops; /* Get next bishop (Isolate LSB) */
        bishop_index = bitscan(position); /* Get index of bishop */
        move_set = magic_bishop_moves(bishop_index, own_mask, enemy_mask); /* Get the move set */
        add_bishop_moves(move_set, board, move_list, bishop_index, enemy_mask); /* Add the moves from this move set to the move list */
        bishops ^= position; /* Reset LSB */
    }
}
void add_bishop_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int from, U64 enemy_mask) {
    /* Adds all the bishop moves from a move set to the move list */
    int side = board->side; /* Makes code writing easier */
    // Declare for loop
    U64 position;
    U64 cap_flag; /* Check if move is a capture */
    int pos_index;
    int cap_piece; /* Captured piece (if any) */
    move_t move; /* Current move */
    while (move_set) { /* Loop through all bishop moves in move set */
        position = move_set & -move_set; /* Isolate LSB */
        pos_index = bitscan(position); /* Get the index of the move */
        cap_flag = position & enemy_mask; /* Check if this is a capture move */
        if (cap_flag) cap_piece = get_captured_piece(board, position, side); /* Get captured piece id (if there is one) */
        move = set_move(from, pos_index /* to */, (side) ? bishop_w : bishop_b /* Piece */, (cap_flag) ? cap_piece : 0 /* Captured piece id */);
        // Set flags
        if (cap_flag) move |= MM_CAP; /* If this is a capture move, set the capture flag */
        // Next
        add_move_to_list(move_list, move); /* Add move to list (obviously) */
        move_set ^= position; /* Remove move from set (Reset LSB) */
    }
}
