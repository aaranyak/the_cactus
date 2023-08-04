/* header for bitboard_utils.c */
#ifndef BOARDUTILS_H
#define BOARDUTILS_H
void render_board(Bitboard *board);
void init_board(Bitboard *board, char init_state[64], int side_to_move);
#endif
