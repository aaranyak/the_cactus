/* init_magics.c
 * Generation of magic bitboard tables.
*/
#include <stdio.h>
#include "lookup_tables.h"
#include "bitboards.h"
#include "bitboard_utils.h"
U64 rook_attacks[64][4096]; /* Rook attack table */
U64 bishop_attacks[64][4096]; /* Bishop attack table */

// Slow rook move gen
U64 rook_attack_loop(int square, U64 blockers) {
    /* Generate rook attacks */
    U64 attacks = 0;
    int debug = 0;
    int rank = square / 8, file = square % 8, p; /* Rank/File variables */
    if (square == 63 && blockers == 0) debug = 1;
    // Ray attacks
    for (p = rank + 1; p <= 7; p++) { /* Loop through all the squares until reached edge */
        attacks |= 1ULL << (p * 8 + file); /* Add the position */
        if (blockers & (1ULL << (p * 8 + file))) break; /* If blocked, break */
    }
    for (p = rank - 1; p >= 0; p--) { /* Do same thing as above, but backwards */
        attacks |= 1ULL << (p * 8 + file);
        if (blockers & (1ULL << (p * 8 + file))) break;
    }
    for (p = file + 1; p <= 7; p++) { /* Ditto, but sideways */
        attacks |= 1ULL << (rank * 8 + p);
        if (blockers & (1ULL << (rank * 8 + p))) break;
    }
    for (p = file - 1; p >= 0; p--) { /* You see what we are getting at */
        attacks |= 1ULL << (rank * 8 + p);
        if (blockers & (1ULL << (rank * 8 + p))) break;
    }
    return attacks;
}
U64 bishop_attack_loop(int square, U64 blockers) {
    /* Generates the bishop attacks using a loop */
    U64 attacks = 0; /* Bishop Attacks */
    int rank = square/8, file = square%8, x, y; /* Declare from before */
    // Ray Attacks.
    for (x = file + 1, y = rank + 1; y <= 7 && x <= 7; x++, y++) {
        attacks |= 1ULL << (x + y*8);
        if (blockers & (1ULL << (x + y*8))) break;
    }
    for (x = file - 1, y = rank + 1; y <= 7 && x >= 0; x--, y++) {
        attacks |= 1ULL << (x + y*8);
        if (blockers & (1ULL << (x + y*8))) break;
    }
    for (x = file + 1, y = rank - 1; y >= 0 && x <= 7; x++, y--) {
        attacks |= 1ULL << (x + y*8);
        if (blockers & (1ULL << (x + y*8))) break;
    }
    for (x = file - 1, y = rank - 1; y >= 0 && x >= 0; x--, y--) {
        attacks |= 1ULL << (x + y*8);
        if (blockers & (1ULL << (x + y*8))) break;
    }
    return attacks;
}
// Initialize attack tables

void init_rook_square_table(int square) {
    /* Initialize the rook attack tables for a square */
    U64 magic = rook_magics[square]; /* Get magic */
    U64 mask = rook_masks[square]; /* Get mask */
    int shift = rook_shifts[square]; /* I hope these repitative comments aren't getting annoying */
    U64 occupancies = 0; /* The blocker bitset */
    int index; /* Magic index */
    U64 attacks; /* Rook attacks */
    do { /* Loop through all subsets of the mask */
        index = (occupancies * magic) >> shift; /* Calculate index */
        attacks = rook_attack_loop(square, occupancies); /* Get the rook attacks */
        rook_attacks[square][index] = attacks; /* Set the attacks in the correct index in the rook attack table */
    } while (occupancies = (occupancies - mask) & mask); /* Somehow this actually works */
    /* Finally...  */
}

void init_bishop_square_table(int square) {
    /* Initialize the bishop attack tables for a square */
    U64 magic = bishop_magics[square]; /* Get magic */
    U64 mask = bishop_masks[square]; /* Get mask */
    int shift = bishop_shifts[square]; /* I hope these repitative comments aren't getting annoying */
    U64 occupancies = 0; /* The blocker bitset */
    int index; /* Magic index */
    U64 attacks; /* Bishop attacks */
    do { /* Loop through all subsets of the mask */
        index = (occupancies * magic) >> shift; /* Calculate index */
        attacks = bishop_attack_loop(square, occupancies); /* Get the bishop attacks */
        bishop_attacks[square][index] = attacks; /* Set the attacks in the correct index in the bishop attack table */
    } while (occupancies = (occupancies - mask) & mask); /* Somehow this actually works */
    /* Finally...  */
}

void init_magic_tables() {
    /* Didn't you read the last line?? */
    for (int square = 0; square < 64; square++) {
        init_rook_square_table(square);  
        init_bishop_square_table(square);
    }
} 
