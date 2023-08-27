/* header file for init_magics.c */
#ifndef INIT_MAGICS_H
#define INIT_MAGICS_H
void test_rook_table(int square);
void init_rook_square_table(int square);
U64 rook_attack_loop(int square, U64 blockers);
extern U64 rook_attacks[64][4096];
extern U64 bishop_attacks[64][4096];
void init_magic_tables();
#endif
