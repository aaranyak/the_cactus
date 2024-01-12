/* Header file for move_gen_utils.c */
#ifndef MOVEGENUTILS_H
#define MOVEGENUTILS_H
U64 colour_mask(Bitboard *board, int side);
int get_captured_piece(Bitboard *board, U64 position, int side);
U64 square_attacked_by(Bitboard *board, int square, U64 occupied);
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))
#endif
