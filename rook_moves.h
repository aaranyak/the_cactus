/* header file for rook_moves.c */
#ifndef MOVEGEN_ROOKMOVES_H
#define MOVEGEN_ROOKMOVES_H
void generate_rook_moves(move_list_t *move_list, Bitboard *board);
U64 magic_rook_moves(int square, U64 own, U64 enemy);

#endif
