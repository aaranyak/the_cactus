/* pawn_moves.c
 * Contains function to generate pseudo-legal pawn moves
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "lookup_tables.h"
#include "move_gen_utils.h"

void add_pawn_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int pawn_index, U64 promotion_check, U64 enpas_check, U64 encap_check); /* Forward decleration */

void generate_pawn_moves(move_list_t *move_list, Bitboard *board) {
    /* Generates all pawn moves from a position, and adds them to move list */
    // Masks
    int side = board->side; /* The side to move */
    U64 own_mask = colour_mask(board, side); /* Generate colour mask for own side */
    U64 enemy_mask = colour_mask(board, !side); /* Generate colour mask for enemy side */
    // Declare for loop.
    U64 pawns = (side) ? board->pieces[pawn_w] : board->pieces[pawn_b]; /* Get the pawn bitboard for the respective side */
    U64 position = 0; /* Current pawn */
    U64 move_set = 0; /* Pawn moves for current pawn */
    move_t move; /* Current move */
    int pawn_index; /* Index of pawn */
    // Check for next move.
    U64 double_push; /* Double pawn push check */
    U64 promotion_check; /* Check if pawn can be promoted */
    U64 block_check; /* Single push check (used for checking if double push is possible */
    U64 enpas_move; /* Check for an en-passant capture */
    // Main generation loop
    while (pawns) { /* Loop through every pawn */
        position = pawns & -pawns; /* Get next pawn (Isolate LSB) */
        pawn_index = bitscan(position); /* Get index of pawn bit */
        // Generate move set
        move_set = (side) ? pawn_attacks_w[pawn_index] : pawn_attacks_b[pawn_index]; /* Lookup pawn capture moves */
        move_set &= enemy_mask; /* Remove non-capture moves from lookup */
        // Pawn pushes.
        if (side) { /* for white */
            block_check = (position << 8) /* move pawn up */ & ~(enemy_mask | own_mask) /* If not blocked */;
            double_push = (block_check) /* If not blocked */ ? ((position << 16) /* Shift up */ & ranks[24] /* Check for 4'th rank */) /* Check for blockers */ & ~(enemy_mask | own_mask) : 0; /* Add double pawn pushing */
            move_set |= block_check | double_push; /* Add them to move set */
        } else { /* For black pieces */
            block_check = (position >> 8) /* move pawn up */ & ~(enemy_mask | own_mask) /* If not blocked */;
            double_push = (block_check) /* If not blocked */ ? ((position >> 16) /* Shift up */ & ranks[32] /* Check for 5'th rank */) /* Check for blockers */ & ~(enemy_mask | own_mask) : 0; /* Add double pawn pushing */
            move_set |= block_check | double_push; /* Add them to move set */
        }
        // Check for en passant
        enpas_move = (side) ? pawn_attacks_w[pawn_index] : pawn_attacks_b[pawn_index]; /* Lookup pawn capture moves */
        enpas_move &= board->enpas; /* Check if the move is an en passant move */
        if (!side) enpas_move &= ranks[16]; /* Rank check */
        else enpas_move &= ranks[40]; /* Ditto */
        move_set |= enpas_move; /* Add en-passant captures to move list */
        // Check for promotion
        promotion_check = (side) ? move_set & ranks[56] : move_set & ranks[0]; /* Check if promotion is possible */
        // Add all the moves to the move list
        add_pawn_moves(move_set, board, move_list, pawn_index, promotion_check, double_push, enpas_move); /* Add all the pawn moves to the move list */
        // Reset LSB
        pawns ^= position; /* Reset the LSB (remove the pawn from the pawns list */
    }
}

void add_pawn_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int pawn_index, U64 promotion_check, U64 enpas_check, U64 encap_check) {
    /* Add pawn moves to the move list from a move bitboard */
    int side = board->side; /* Makes code writing easier */
    U64 enemy_mask = colour_mask(board, !side); /* Get the oppoenent mask */
    // Declare for loop
    U64 position;
    U64 cap_flag; /* Check if move is a capture */
    int pos_index;
    int cap_piece; /* Captured piece (if any) */
    move_t move; /* Current move */
    move_t rook_pro, knight_pro, bishop_pro, queen_pro; /* Use for promotion */
    while (move_set) { /* Loop through the moves */
        position = move_set & -move_set; /* Get next move (Isolate lsb) */
        pos_index = bitscan(position); /* Get the index of the position */
        // Create basic move
        cap_flag = position & enemy_mask; /* Check if move is a capture */
        if (cap_flag) cap_piece = get_captured_piece(board, position, side); /* If this is a capture, check which piece is being captured */
        // Create move and add move
        move = set_move(pawn_index /* from */, pos_index /* to */, (side) ? pawn_w : pawn_b /* piece type */,(cap_flag) ? cap_piece : 0 /* captured piece id */); /* Create a new move code */
        if (cap_flag) move |= MM_CAP; /* If this is a capture move, add the capture flag to the move */
        // Enpas Capture
        if (position & encap_check) move |= MM_EPC; /* Make this an en passant capture move if it is one */
        // Double pawn push
        if (position & enpas_check) move |= MM_DPP; /* Make this a double pawn push if it is one */
        // Add promotion moves.
        if (promotion_check) {
            // Create a promotion move
            rook_pro = move; /* Set the rook promotion move to the same as this move */
            rook_pro |= MM_PRO; /* Make this a promotion move */
            // Set all other promotions
            knight_pro = bishop_pro = queen_pro = rook_pro; /* Set all the other promotion moves */
            // Set the promotion types
            /* rook promotion is of a type 0 */
            knight_pro |= knight_w << MS_PPP;
            bishop_pro |= bishop_w << MS_PPP;
            queen_pro |= queen_w << MS_PPP;
            // Add promotion moves
            add_move_to_list(move_list, rook_pro); /* Add */
            add_move_to_list(move_list, knight_pro); /* Add */
            add_move_to_list(move_list, bishop_pro); /* Add */
            add_move_to_list(move_list, queen_pro); /* Add */
        }
        add_move_to_list(move_list, move); /* Add move to move list */
        move_set ^= position; /* Reset LSB (remove bit from list */
    }
}
