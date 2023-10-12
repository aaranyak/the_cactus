#ifndef PERFTTEST_H
#define PERFTTEST_H
int count_moves(Bitboard *board, int depth); /* Forward declaration */
void do_test(Bitboard *board, int maxdepth); /* Ditto */
void play_game(Bitboard *board, int side);
#endif
