/* header file for legality_test.c */
#ifndef LEGALITYTEST_H
#define LEGALITYTEST_H
#include "moves.h"
#include "bitboards.h"
U64 pawn_attack_mask(Bitboard *board, int side);
U64 knight_attack_mask(Bitboard *board, int side);
U64 king_attack_mask(Bitboard *board, int side);
U64 rook_attack_mask(Bitboard *board, int side, U64 own, U64 enemy);
U64 bishop_attack_mask(Bitboard *board, int side, U64 own, U64 enemy);
U64 queen_attack_mask(Bitboard *board, int side, U64 own, U64 enemy);
int is_check(Bitboard *board, int side);
int is_legal(Bitboard *board, move_t move);
void update_sliding_piece_attacks(Bitboard *board);
void update_attack_table(Bitboard *board, int piece);
#endif
