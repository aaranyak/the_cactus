/* knight moves.c
 * Contains functions t generate all knight moves and add them to moves list.
 *  -> Move generation by lookup
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "lookup_tables.h"
#include "move_gen_utils.h"

void add_knight_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int knight_index, U64 enemy_mask); /* Forward decleration */

void generate_knight_moves(move_list_t *move_list, Bitboard *board) {
    /* Generates all knight moves and adds them to move list */
    // Masks
    int side = board->side; /* Side to move */
    U64 own_mask = colour_mask(board, side); /* Piece mask of own side */
    U64 enemy_mask = colour_mask(board, !side); /* Piece mask of enemy pieces */
    // Declare for loop
    U64 knights = (side) ? board->pieces[knight_w] : board->pieces[knight_b]; /* Knights bitboard */
    U64 position = 0; /* Knight position (current bit) */
    U64 move_set = 0; /* Set of knight moves */
    int knight_index; /* Index of current knight */
    
    while (knights) { /* Loop through every knight on the board */
        position = knights & -knights; /* Get next knight */
        knight_index = bitscan(position); /* Get the index of the knight */
        move_set = knight_attacks[knight_index]; /* Lookup knight moves */
        move_set &= ~own_mask; /* Remove blocked squares */
        add_knight_moves(move_set, board, move_list, knight_index, enemy_mask); /* Add all moves to moves list */
        // Reset LSB
        knights ^= position; /* Move to next knight */
    }
}

void add_knight_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int knight_index, U64 enemy_mask) {
    /* Add all knight moves to move list */
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
        move = set_move(knight_index /* from */, pos_index /* to */, (side) ? knight_w : knight_b /* Piece */, (cap_flag) ? cap_piece : 0 /* Captured Piece */); /* Set the move */
        // Set flags
        if (cap_flag) move |= MM_CAP; /* If this is a capture move, set the capture flag */
        // Next
        add_move_to_list(move_list, move); /* Add move to list (obviously) */
        move_set ^= position; /* Remove move from set (Reset LSB) */
    }
}
