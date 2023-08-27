/* header file for bishop_moves.c */
#ifndef MOVEGEN_BISHOPMOVES_H
#define MOVEGEN_BISHOPMOVES_H
void generate_bishop_moves(move_list_t *move_list, Bitboard *board);
U64 magic_bishop_moves(int square, U64 own, U64 enemy);

#endif
