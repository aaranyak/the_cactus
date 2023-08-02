#ifndef BITBOARDS_H /* Header Guard */
#define BITBOARDS_H
#include <stdint.h> /* for uint64_t */

typedef uint64_t U64; /* Type for all 64 bit unsigned integers */

// Define structure for bitboards
typedef struct Bitboard {
    U64 pieces[12]; /* All piece bitboards */
    // Other board details
    
    int side; /* Side to move */
    U64 key; /* Zobrist hash for bitboard */
} Bitboard;

// Other important details
enum { /* Piece ids */
    rook_w = 0, knight_w = 1, bishop_w = 2, queen_w = 3, king_w = 4, pawn_w = 5, 
    rook_b = 6, knight_b = 7, bishop_b = 8, queen_b = 9, king_b = 10, pawn_b = 11
};

#endif
