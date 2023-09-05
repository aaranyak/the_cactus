/* header file for legality_test.c */
#ifndef LEGALITYTEST_H
#define LEGALITYTEST_H
U64 pawn_attack_mask(Bitboard *board, int side);
U64 knight_attack_mask(Bitboard *board, int side);
U64 king_attack_mask(Bitboard *board, int side);
U64 rook_attack_mask(Bitboard *board, int side, U64 own, U64 enemy);
U64 bishop_attack_mask(Bitboard *board, int side, U64 own, U64 enemy);
U64 queen_attack_mask(Bitboard *board, int side, U64 own, U64 enemy);
int is_check(Bitboard *board, int side);
int is_legal(Bitboard *board, move_t move);
#endif
