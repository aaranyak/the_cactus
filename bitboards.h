#ifndef BITBOARDS_H /* Header Guard */
#define BITBOARDS_H
#include <stdint.h> /* for uint64_t */

typedef uint64_t U64; /* Type for all 64 bit unsigned integers */

// Define structure for bitboards
typedef struct Bitboard {
    U64 pieces[12]; /* All piece bitboards */
    // Other board details
    U64 castling_rights; /* 1st 4 bits are relevant */
    U64 enpas; /* Figure this out later (probably a file mask) */
    U64 attack_tables[12]; /* Attack tables of all the pieces on the board */
    int side; /* Side to move */
    
    int piece_square_eval; /* Evaluation term for piece-square-tables */
    U64 key; /* Zobrist hash for bitboard */
    int moves;
} Bitboard;

// Other important details
enum { /* Piece ids */
    rook_w = 0, knight_w = 1, bishop_w = 2, queen_w = 3, king_w = 4, pawn_w = 5, 
    rook_b = 6, knight_b = 7, bishop_b = 8, queen_b = 9, king_b = 10, pawn_b = 11
};

// Castling right masks.
#define WK_CASTLE 1 /* & it with the castling rights to get the value */
#define WQ_CASTLE 2
#define BK_CASTLE 4
#define BQ_CASTLE 8
// Full masks
#define W_CASTLE 3
#define B_CASTLE 12

// Bitwise operation hardware instructions.
#define bitscan(x) __builtin_ctzll(x) /* Bitscan Forward */
#define popcount(x) __builtin_popcountll(x) /* Population count of bitset */
#define cutoff(x) (((x) > 0) ? (x) : 0)
#endif
