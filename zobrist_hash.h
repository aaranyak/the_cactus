/* Header file for zobrist_hash.h */
#ifndef ZOBRIST_H
#define ZOBRIST_H
extern U64 pst_hash[12][64]; /* Random numbers for pieces on square */
extern U64 side_hash; /* Random number for side to move is black*/
extern U64 cr_hash[4]; /* Random numbers for castling rights */
extern U64 epf_hash[8]; /* Random numbers for ep-file*/
void init_hash_keys();
void update_key_castle(Bitboard *board, int side, int cas_side);
void update_key_prom(Bitboard *board, int piece, int from, int to, int type, int cap, int cap_piece);
void update_key_ep(Bitboard *board, int piece, int from, int to, int cap_square, int cap_piece);
void update_key_move(Bitboard *board, int piece, int from, int to, int cap, int cap_piece);
#endif
