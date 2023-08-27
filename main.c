/* The Cactus
 *  -> Minmax Search with alpha-beta pruning
 *  -> Iterative deepening method
 *  -> Written in C
*/

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
#include "lookup_tables.h"
#include "init_magics.h"
#include "make_move.h"
int main(int argc, char **argv) {
    /* Run chess engine */

    // The board state to start with
    char initial_state[64] = {
        'r','n','b','q','k','b','n','r',
        'p','p','p','p','p','p','p','p',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        'P','P','P','P','P','P','P','P',
        'R','N','B','Q','K','B','N','R',
    }; /* An Array of characters as the starting board state */

    char test_position[64] = {
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        'p','P',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ','P','P','P','P','P','P',
        'R','N','B','Q','K','B','N','R',
    }; /* Starting position to test board states on */
    
    // Init board
    Bitboard board; /* Allocate space for bitboard */
    
    init_board(&board, test_position, 0); /* Initialize board */
    printf("The Cactus - a chess playing AI that is supposed to defeat humans.\n\n"); /* Opening Message */
    // Test magic generation
    init_magic_tables();
    board.enpas = files[1];
    U64 enpas_file, castling_rights, key; /* Just testing */
    move_t move = set_move(24, 17, pawn_b, 0); /* Test Move Making */
    move |= MM_EPC; /* En-passant capture move */
    render_board(&board); /* Previous state */
    make_move(&board, move, &enpas_file, &castling_rights, &key);
    render_board(&board); /* Display board */
    unmake_move(&board, move, &enpas_file, &castling_rights, &key);
    render_board(&board); /* Display board */
    return 0;
}
