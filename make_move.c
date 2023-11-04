/* make_move.c *
 * This file contains the function to make/unmake the move on the board
 * The make move function requires to store some extra data for the unmake function later on.
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
#include "legality_test.h"
#include "zobrist_hash.h"

/* Castling move macros */
// White King-side Castling
#define CAS_KING_WK 0x0000000000000050
#define CAS_ROOK_WK 0x00000000000000A0
// White Queen-side Castling
#define CAS_KING_WQ 0x0000000000000014
#define CAS_ROOK_WQ 0x0000000000000009
// Black King-side Castling
#define CAS_KING_BK 0x5000000000000000
#define CAS_ROOK_BK 0xA000000000000000
// Black Queen-side Castling
#define CAS_KING_BQ 0x1400000000000000
#define CAS_ROOK_BQ 0x0900000000000000

void make_move(Bitboard *board, move_t move, U64 *enpas_file, U64 *castling_rights, U64 *key /* Better just save this rather than incremental update, saves more time writing code */, int *piece_square_eval) {
    /* Make the move on the move structure on the bitboard */
    // Set saved values for unmake
    int side = board->side; /* Convenience reasons */
    *enpas_file = board->enpas; /* Set the en passant file */
    *castling_rights = board->castling_rights; /* Set the old castling rights */
    *key = board->key; /* Save programming time */
    *piece_square_eval = board->piece_square_eval; /* Save the evaluation term for piece-square tables */
    
    // Handle castling moves
    if (move & MM_CAS) { /* If this is a castling move */
        /* Handle Castling */
        if (side) { /* If white is castling */
            board->pieces[king_w] ^= (move & MM_CSD) ? CAS_KING_WQ : CAS_KING_WK; /* Move the king */
            board->pieces[rook_w] ^= (move & MM_CSD) ? CAS_ROOK_WQ : CAS_ROOK_WK; /* Move the rook */
        } else { /* If black is castling */
            board->pieces[king_b] ^= (move & MM_CSD) ? CAS_KING_BQ : CAS_KING_BK; /* Move the king */
            board->pieces[rook_b] ^= (move & MM_CSD) ? CAS_ROOK_BQ : CAS_ROOK_BK; /* Move the rook */
        }
        board->enpas = 0; /* Disable en-passant capture */

        // Update attack tables
        update_attack_table(board, side ? king_w : king_b); /* Update king attack table */
        update_sliding_piece_attacks(board); /* Update the sliding piece attack table, since that can be affected by moving blockers */
        
        // Update piece-square tables
        U64 cas_side = move & MM_CSD;
        if (side) board->piece_square_eval += cas_side ? 15 : 35; /* Add the castling advantage to the pst*/
        else board->piece_square_eval -= cas_side ? 15 : 35; /* Add the castling advantage to the pst*/
        update_key_castle(board, side, cas_side); /* Update the zobrist hash key while castling */
        board->castling_rights &= ~(side ? W_CASTLE : B_CASTLE); /* Update castling rights */
        board->side = !board->side; /* Toggle side-to-move */
        board->moves++; /* Plus plus the move count */
        return;
    }
    // Get move data
    int from = move & MM_FROM; /* Get the from square */
    int to = (move & MM_TO) >> MS_TO; /* Get the to square */
    int piece = (move & MM_PIECE) >> MS_PIECE; /* Piece type id */
    int cap_piece = (move & MM_EAT) >> MS_EAT; /* Captured piece id */
    int promoted = (move & MM_PPP) >> MS_PPP; /* Pawn promoted to piece type */
    
    // Do the actual moving
    // Pawn promotion Move
    if (move & MM_PRO) { /* If this is a pawn promotion move */
        board->pieces[piece] ^= 1ULL << from; /* Remove piece */
        board->pieces[board->side ? promoted : promoted + 6] ^= 1ULL << to; /* Appear as promoted piece */
        if (move & MM_CAP) /* If this is a capture move */ board->pieces[cap_piece] ^= 1ULL << to; /* Remove captured piece from board */
        // Update zobrist key
        update_key_prom(board, piece, from, to, board->side ? promoted : promoted + 6, move & MM_CAP, cap_piece); /* Update the zobrist hash */ 
        // Update Piece-square Tables
        board->piece_square_eval -= piece_square[piece][from]; /* Remove from-square */
        board->piece_square_eval += piece_square[board->side ? promoted : promoted + 6][to]; /* Add promoted piece on to-square */
        if (move & MM_CAP) board->piece_square_eval -= piece_square[cap_piece][to]; /* Remove captured piece (if so) */
    }
    // En-passant capture
    else if (move & MM_EPC) { /* If this is an en-passant capture move */
        board->pieces[piece] ^= (1ULL << from) | (1ULL << to); /* Move the piece to the correct square */
        U64 ep_cap_pos = ranks[side ? 32 : 24] /* Capture rank */ & board->enpas; /* Capture file */
        board->pieces[(side) ? pawn_b : pawn_w] ^= ep_cap_pos; /* Remove the en-passant capture piece */
        // Update zobrist key
        update_key_ep(board, piece, from, to, bitscan(ep_cap_pos), side ? pawn_b : pawn_w);

        // Update Piece-square Tables
        board->piece_square_eval -= piece_square[piece][from]; /* Remove from-square */
        board->piece_square_eval += piece_square[piece][to]; /* Add to--square */
        board->piece_square_eval -= piece_square[side ? pawn_b : pawn_w][bitscan(ep_cap_pos)]; /* Remove ep-captured piece */
    }
    // Normal Move
    else { /* Finally, a normal move... */
        board->pieces[piece] ^= (1ULL << from) | (1ULL << to); /* Move the piece to the correct square */
        if (move & MM_CAP) /* If this is a capture move */ board->pieces[cap_piece] ^= 1ULL << to; /* Remove captured piece from board */
        // Update zobrist key
        update_key_move(board, piece, from, to, move & MM_CAP, cap_piece); /* look in zobrist_hash.c */
        // Update Piece-square Tables
        board->piece_square_eval -= piece_square[piece][from]; /* Remove from-square */
        board->piece_square_eval += piece_square[piece][to]; /* Add to-square */
        if (move & MM_CAP) board->piece_square_eval -= piece_square[cap_piece][to]; /* Remove captured piece (if so) */
    }
    // Set en-passant file
    
    // Remove the old ep-file from the hash key
    int old_ep = bitscan(*enpas_file & 255); /* Get the previous en-passant file */
    if (*enpas_file) board->key ^= epf_hash[old_ep]; /* If there is an old ep file, remove it */
   
    if (move & MM_DPP) { /* If this is a double pawn push */
        board->enpas = files[to]; /* Set the en-passant file to the to move */
        
        // Update zobrist hash key
        board->key ^= epf_hash[to % 8]; /* Add the new en-passant file */
    }
    else board->enpas = 0; /* Otherwise, no en-passant file */

    // Update attack tables
    update_attack_table(board, piece); /* Update the attack table of the moved piece */
    if (move & MM_CAP) update_attack_table(board, cap_piece); /* If this is a capture move, update the attack table of the captured piece */
    if (move & MM_EPC) update_attack_table(board, side ? pawn_b : pawn_w); /* If this is an ep capture move, update the opponent pawn attack table */
    if (move & MM_PRO) update_attack_table(board, side ? promoted : promoted + 6); /* If this is a pawn promotion, update the promoted piece attack table */
    update_sliding_piece_attacks(board); /* Update the sliding piece attack table, since that can be affected by moving blockers */

    // Set castling rights
    if (piece == rook_w && from == 7) { /* King side rook move */
        if (board->castling_rights & WK_CASTLE) board->key ^= cr_hash[0]; /* Update hash */
        board->castling_rights &= ~WK_CASTLE; /* If king-side rook is moved, disable king-side castling */
    } if (piece == rook_w && from == 0) { /* Queen side rook move */
        if (board->castling_rights & WQ_CASTLE) board->key ^= cr_hash[1]; /* Update hash */
        board->castling_rights &= ~WQ_CASTLE; /* If queen-side rook is moved, disable queen-side castling */
    } if (piece == king_w) { /* King move */
        if (board->castling_rights & WK_CASTLE) board->key ^= cr_hash[0]; /* Update hash */
        if (board->castling_rights & WQ_CASTLE) board->key ^= cr_hash[1]; /* Update hash */
        board->castling_rights &= ~W_CASTLE; /* If king is moved, disable castling */
    } // Ditto for black
    if (piece == rook_b && from == 63) { /* King side rook move */
        if (board->castling_rights & BK_CASTLE) board->key ^= cr_hash[3]; /* Update hash */
        board->castling_rights &= ~BK_CASTLE; /* If king-side rook is moved, disable king-side castling */
    } if (piece == rook_b && from == 56) { /* Queen side rook move */
        if (board->castling_rights & BQ_CASTLE) board->key ^= cr_hash[4]; /* Update hash */
        board->castling_rights &= ~BQ_CASTLE; /* If queen-side rook is moved, disable queen-side castling */
    } if (piece == king_b) { /* King move */
        if (board->castling_rights & BK_CASTLE) board->key ^= cr_hash[3]; /* Update hash */
        if (board->castling_rights & BQ_CASTLE) board->key ^= cr_hash[4]; /* Update hash */
        board->castling_rights &= ~B_CASTLE; /* If king is moved, disable castling */
    }
    // Change side-to-move
    board->side = !board->side; /* Toggle this */
    board->key ^= side_hash; /* Toggle side-to-move on zobrist key */
    board->moves++; /* Plus plus the move count */
}


void unmake_move(Bitboard *board, move_t move, U64 *enpas_file, U64 *castling_rights, U64 *key, int *piece_square_eval) {
    /* Unmakes the move on the board */
    // Reset saved values
    board->enpas = *enpas_file;
    board->castling_rights = *castling_rights;
    board->key = *key;
    board->piece_square_eval = *piece_square_eval; 
    // Since the xor operation is it's own inverse, we can just repeat the same steps we used for the make move function.

    // Change the side-to-move
    board->side = !board->side; /* Toggle side-to-move */
    int side = board->side; /* Why not */
    board->moves--; /* Minus Minus */

    // Handle castling moves
    if (move & MM_CAS) { /* If this is a castling move */
        /* Handle Castling */
        if (side) { /* If white is castling */
            board->pieces[king_w] ^= (move & MM_CSD) ? CAS_KING_WQ : CAS_KING_WK; /* Move the king */
            board->pieces[rook_w] ^= (move & MM_CSD) ? CAS_ROOK_WQ : CAS_ROOK_WK; /* Move the rook */
        } else { /* If black is castling */
            board->pieces[king_b] ^= (move & MM_CSD) ? CAS_KING_BQ : CAS_KING_BK; /* Move the king */
            board->pieces[rook_b] ^= (move & MM_CSD) ? CAS_ROOK_BQ : CAS_ROOK_BK; /* Move the rook */
        }
      
        // Update attack tables
        update_attack_table(board, side ? king_w : king_b); /* Update king attack table */
        update_sliding_piece_attacks(board); /* Update the sliding piece attack table, since that can be affected by moving blockers */ 
        return;
    }

    // Get move data
    int from = move & MM_FROM; /* Get the from square */
    int to = (move & MM_TO) >> MS_TO; /* Get the to square */
    int piece = (move & MM_PIECE) >> MS_PIECE; /* Piece type id */
    int cap_piece = (move & MM_EAT) >> MS_EAT; /* Captured piece id */
    int promoted = (move & MM_PPP) >> MS_PPP; /* Pawn promoted to piece type */
    
    // Do the actual moving
    // Pawn promotion Move
    if (move & MM_PRO) { /* If this is a pawn promotion move */
        board->pieces[piece] ^= 1ULL << from; /* Remove piece */
        board->pieces[board->side ? promoted : promoted + 6] ^= 1ULL << to; /* Appear as promoted piece */
        if (move & MM_CAP) /* If this is a capture move */ board->pieces[cap_piece] ^= 1ULL << to; /* Remove captured piece from board */
    }
    // En-passant capture
    else if (move & MM_EPC) { /* If this is an en-passant capture move */
        board->pieces[piece] ^= (1ULL << from) | (1ULL << to); /* Move the piece to the correct square */
        U64 ep_cap_pos = ranks[side ? 32 : 24] /* Capture rank */ & board->enpas; /* Capture file */
        board->pieces[(side) ? pawn_b : pawn_w] ^= ep_cap_pos; /* Remove the en-passant capture piece */
    }
    // Normal Move
    else { /* Finally, a normal move... */
        board->pieces[piece] ^= (1ULL << from) | (1ULL << to); /* Move the piece to the correct square */
        if (move & MM_CAP) /* If this is a capture move */ board->pieces[cap_piece] ^= 1ULL << to; /* Remove captured piece from board */
    }
    
    // Update attack tables
    update_attack_table(board, piece); /* Update the attack table of the moved piece */
    if (move & MM_CAP) update_attack_table(board, cap_piece); /* If this is a capture move, update the attack table of the captured piece */
    if (move & MM_EPC) update_attack_table(board, side ? pawn_b : pawn_w); /* If this is an ep capture move, update the opponent pawn attack table */
    if (move & MM_PRO) update_attack_table(board, side ? promoted : promoted + 6); /* If this is a pawn promotion, update the promoted piece attack table */
    update_sliding_piece_attacks(board); /* Update the sliding piece attack table, since that can be affected by moving blockers */
   
    return;
}
