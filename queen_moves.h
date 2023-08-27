/* header file for queen_moves.c */
#ifndef MOVEGEN_QUEENMOVES_H
#define MOVEGEN_QUEENMOVES_H
void generate_queen_moves(move_list_t *move_list, Bitboard *board);
#define magic_queen_moves(square, own_mask, enemy_mask) (magic_rook_moves(square, own_mask, enemy_mask) | magic_bishop_moves(square, own_mask, enemy_mask)) /* Macro for queen magic looku. Basically | of rook and bishop moves */
#endif
