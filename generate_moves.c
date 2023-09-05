/* Wrapper for move generation */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "pawn_moves.h"
#include "king_moves.h"
#include "knight_moves.h"
#include "rook_moves.h"
#include "bishop_moves.h"
#include "queen_moves.h"
#include "castling_moves.h"
#include "move_gen_utils.h"

void generate_moves(Bitboard *board, move_list_t *moves) {
    /* Generate all pseudo-legal moves */
        generate_pawn_moves(moves, board);
        generate_knight_moves(moves, board);
        generate_king_moves(moves, board);
        generate_rook_moves(moves, board);
        generate_bishop_moves(moves, board);
        generate_queen_moves(moves, board);
        generate_castling_moves(moves, board);
}
