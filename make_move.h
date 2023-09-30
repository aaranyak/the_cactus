/* header file for make_move.c */
#ifndef MAKEMOVE_H
#define MAKEMOVE_H
void make_move(Bitboard *board, move_t move, U64 *enpas_file, U64 *castling_rights, U64 *key, int *piece_squar_eval);
void unmake_move(Bitboard *board, move_t move, U64 *enpas_file, U64 *castling_rights, U64 *key, int *piece_square_eval);
#endif
