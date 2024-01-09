/* Header file for move_ordering.c */
#ifndef MOVEORDERING_H
#define MOVEORDERING_H
void order_moves(move_list_t *move_list, Bitboard *board, int use_hash_move, move_t hash_move, int depth);
void sort_moves(move_list_t *move_list, int scores[]);
#endif
