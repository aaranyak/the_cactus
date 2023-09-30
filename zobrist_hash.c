/* Generate a 64 bit hash key to use in lookup tables.
 * Uses Zobrist Hashing method
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

U64 pst_hash[12][64]; /* Random numbers for pieces on square */
U64 side_hash; /* Random number for side to move is black*/
U64 cr_hash[4]; /* Random numbers for castling rights */
U64 epf_hash[8]; /* Random numbers for ep-file*/

// Source - Answer on https://stackoverflow.com/a/33021408
uint64_t rand_uint64_slow(void) {
  /* Generate random U64 bit by bit. Written by [Some random guy on stackoverflow] */
  uint64_t r = 0;
  for (int i=0; i<64; i++) {
    r = r*2 + rand()%2;
  }
  return r;
}

void init_hash_keys() {
    /* Initializes all the random numbers used for Zobrist hashing */

    // Initialize piece-square numbers
    for (int piece = 0; piece < 12; piece++) { /* Loop through all the pieces */
        for (int square = 0; square < 64; square++) { /* Loop through all the squares */
            pst_hash[piece][square] = rand_uint64_slow(); /* Use a random uint64 */
        }
    }
    for (int file = 0; file < 8; file++) epf_hash[file] = rand_uint64_slow(); /* Set random numbers for en-passant files */
    for (int right = 0; right < 4; right++) cr_hash[right] = rand_uint64_slow(); /* Set random numbers for castling rights */
    side_hash = rand_uint64_slow(); /* Side to move random number */
}

void update_key_castle(Bitboard *board, int side, int cas_side) {
    /* Update hash key for castling */
    if (side) { /* If white is castling */
        board->key ^= pst_hash[king_w][4]; /* Remove the king from it's place */
        board->key ^= pst_hash[rook_w][cas_side ? 0 : 7]; /* Remove the rook from it's place */
        // Put the stuff in it's place
        board->key ^= pst_hash[king_w][cas_side ? 2 : 6]; /* Put the king in it's place */
        board->key ^= pst_hash[rook_w][cas_side ? 3 : 5]; /* Put the rook in it's place */
        // Update castling rights
        if (board->castling_rights & WK_CASTLE) board->key ^= cr_hash[0];
        if (board->castling_rights & WQ_CASTLE) board->key ^= cr_hash[1];
    } else { /* If black is castling */
        board->key ^= pst_hash[king_w][60]; /* Remove the king from it's place */
        board->key ^= pst_hash[rook_w][cas_side ? 56 : 63]; /* Remove the rook from it's place */
        // Put the stuff in it's place
        board->key ^= pst_hash[king_w][cas_side ? 58 : 62]; /* Put the king in it's place */
        board->key ^= pst_hash[rook_w][cas_side ? 59 : 61]; /* Put the rook in it's place */
        // Update castling rights
        if (board->castling_rights & BK_CASTLE) board->key ^= cr_hash[2];
        if (board->castling_rights & BQ_CASTLE) board->key ^= cr_hash[3];
    }
    board->key ^= side_hash; /* Toggle side-to-move */
}

void update_key_prom(Bitboard *board, int piece, int from, int to, int type, int cap, int cap_piece) {
    /* Update zobrist key for pawn promotion move */
    board->key ^= pst_hash[piece][from]; /* Remove the pawn */
    board->key ^= pst_hash[type][to]; /* Reappear as the new piece! */
    if (cap) board->key ^= pst_hash[cap_piece][to]; /* Remove the captured piece if any */
}

void update_key_ep(Bitboard *board, int piece, int from, int to, int cap_square, int cap_piece) {
    /* Updates the hash key for a en-passant capture */
    board->key ^= pst_hash[piece][from]; /* Remove the pawn */
    board->key ^= pst_hash[piece][to]; /* Put the pawn */
    board->key ^= pst_hash[cap_piece][cap_square]; /* Remove the captured pawn */
}

void update_key_move(Bitboard *board, int piece, int from, int to, int cap, int cap_piece) {
    /* Update zobrist key for normal move */
    board->key ^= pst_hash[piece][from]; /* Remove the piece */
    board->key ^= pst_hash[piece][to]; /* Put the piece */
    if (cap) board->key ^= pst_hash[cap_piece][to]; /* Remove the captured piece if any */
}

