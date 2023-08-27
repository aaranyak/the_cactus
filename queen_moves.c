/* queen_moves.c
 * Generate all possible queen moves
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
#include "rook_moves.h"
#include "bishop_moves.h"

void add_queen_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int from, U64 enemy_mask);

void generate_queen_moves(move_list_t *move_list, Bitboard *board) {
    /* Generates all possible moves by moving queens, and adds them to the move list */
    // Masks
    int side = board->side; /* Side to move */
    U64 own_mask = colour_mask(board, side); /* Piece mask of own side */
    U64 enemy_mask = colour_mask(board, !side); /* Piece mask of enemy pieces */
    // Declare for loop
    U64 queens = (side) ? board->pieces[queen_w] : board->pieces[queen_b]; /* Queens bitboard */
    U64 position = 0; /* Queen position (current bit) */
    U64 move_set = 0; /* Set of queen moves */
    int queen_index; /* Index of current queen */

    while (queens) { /* Loop through every queen position */
        position = queens & -queens; /* Get next queen (Isolate LSB) */
        queen_index = bitscan(position); /* Get index of queen */
        /* Queen move set is a union of the rook move set and the bishop move set */
        move_set = magic_rook_moves(queen_index, own_mask, enemy_mask) | magic_bishop_moves(queen_index, own_mask, enemy_mask); /* Get queen move set using magic bitboard lookup */
        add_queen_moves(move_set, board, move_list, queen_index, enemy_mask); /* Add the moves from this move set to the move list */
        queens ^= position; /* Reset LSB */
    }
}

void add_queen_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int from, U64 enemy_mask) {
    /* Adds all the queen moves from a move set to the move list */
    int side = board->side; /* Makes code writing easier */
    // Declare for loop
    U64 position;
    U64 cap_flag; /* Check if move is a capture */
    int pos_index;
    int cap_piece; /* Captured piece (if any) */
    move_t move; /* Current move */
    while (move_set) { /* Loop through all queen moves in move set */
        position = move_set & -move_set; /* Isolate LSB */
        pos_index = bitscan(position); /* Get the index of the move */
        cap_flag = position & enemy_mask; /* Check if this is a capture move */
        if (cap_flag) cap_piece = get_captured_piece(board, position, side); /* Get captured piece id (if there is one) */
        move = set_move(from, pos_index /* to */, (side) ? queen_w : queen_b /* Piece */, (cap_flag) ? cap_piece : 0 /* Captured piece id */);
        // Set flags
        if (cap_flag) move |= MM_CAP; /* If this is a capture move, set the capture flag */
        // Next
        add_move_to_list(move_list, move); /* Add move to list (obviously) */
        move_set ^= position; /* Remove move from set (Reset LSB) */
    }
}

