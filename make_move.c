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

void make_move(Bitboard *board, move_t move, U64 *enpas_file, U64 *castling_rights, U64 *key /* Better just save this rather than incremental update, saves more time writing code */) {
    /* Make the move on the move structure on the bitboard */
    // Set saved values for unmake
    int side = board->side; /* Convenience reasons */
    *enpas_file = board->enpas; /* Set the en passant file */
    *castling_rights = board->castling_rights; /* Set the old castling rights */
    *key = board->key; /* Save programming time */
    
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
        board->castling_rights &= ~(side ? W_CASTLE : B_CASTLE); /* Update castling rights */
        board->enpas = 0; /* Disable en-passant capture */
        board->side = !board->side; /* Toggle side-to-move */
        return;
    }
    // Get move data
    int from = move & MM_FROM; /* Get the from square */
    int to = (move & MM_TO) >> MS_TO; /* Get the to square */
    int piece = (move & MM_PIECE) >> MS_PIECE; /* Piece type id */
    int cap_piece = (move & MM_EAT) >> MS_EAT; /* Captured piece id */
    
    // Do the actual moving
    // Pawn promotion Move
    if (move & MM_PRO) { /* If this is a pawn promotion move */
        int promoted = (move & MM_PPP) >> MS_PPP; /* Pawn promoted to piece type */
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
    // Set en-passant file
    if (move & MM_DPP) /* If this is a double pawn push */ board->enpas = files[to]; /* Set the en-passant file to the to move */
    else board->enpas = 0; /* Otherwise, no en-passant file */
    
    // Set castling rights
    if (piece == rook_w && from == 7) board->castling_rights &= ~WK_CASTLE; /* If king-side rook is moved, disable king-side castling */
    if (piece == rook_w && from == 0) board->castling_rights &= ~WQ_CASTLE; /* If queen-side rook is moved, disable queen-side castling */
    if (piece == king_w) board->castling_rights &= ~W_CASTLE; /* If king is moved, disable castling */
    // Ditto for black
    if (piece == rook_b && from == 63) board->castling_rights &= ~BK_CASTLE; /* If king-side rook is moved, disable king-side castling */
    if (piece == rook_b && from == 56) board->castling_rights &= ~BQ_CASTLE; /* If queen-side rook is moved, disable queen-side castling */
    if (piece == king_b) board->castling_rights &= ~B_CASTLE; /* If king is moved, disable castling */
    
    // Change side-to-move
    board->side = !board->side; /* Toggle this */
}


void unmake_move(Bitboard *board, move_t move, U64 *enpas_file, U64 *castling_rights, U64 *key) {
    /* Unmakes the move on the board */
    // Reset saved values
    board->enpas = *enpas_file;
    board->castling_rights = *castling_rights;
    board->key = *key;
    
    // Since the xor operation is it's own inverse, we can just repeat the same steps we used for the make move function.

    // Change the side-to-move
    board->side = !board->side; /* Toggle side-to-move */
    int side = board->side; /* Why not */

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
        return;
    }

    // Get move data
    int from = move & MM_FROM; /* Get the from square */
    int to = (move & MM_TO) >> MS_TO; /* Get the to square */
    int piece = (move & MM_PIECE) >> MS_PIECE; /* Piece type id */
    int cap_piece = (move & MM_EAT) >> MS_EAT; /* Captured piece id */
    
    // Do the actual moving
    // Pawn promotion Move
    if (move & MM_PRO) { /* If this is a pawn promotion move */
        int promoted = (move & MM_PPP) >> MS_PPP; /* Pawn promoted to piece type */
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
    return;
}
