/* pawn_moves.h */
#ifndef MOVEGEN_PAWNMOVES_H
#define MOVEGEN_PAWNMOVES_H
void generate_pawn_moves(move_list_t *move_list, Bitboard *board);
void add_pawn_moves(U64 move_set, Bitboard *board, move_list_t *move_list, int pawn_index, U64 promotion_check, U64 enpas_check);
#endif
