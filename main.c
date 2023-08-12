/* The Cactus
 *  -> Minmax Search with alpha-beta pruning
 *  -> Iterative deepening method
 *  -> Written in C
*/

#include <stdio.h>
#include <stdlib.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "pawn_moves.h"
#include "king_moves.h"
#include "move_gen_utils.h"
#include "lookup_tables.h"

Bitboard *board;
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
        'r','n','b','q',' ','b','n','r',
        'p','p','p','p','p',' ','p','p',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ','p',' ',' ',
        ' ',' ',' ',' ','k',' ',' ',' ',
        'P','P','P','P','P','P','P','P',
        'R','N','B','Q','K','B','N','R',
    }; /* Starting position to test board states on */
    
    // Init board
    board = (Bitboard*)malloc(sizeof(Bitboard)); /* Allocate space for bitboard */
    init_board(board, test_position, 0); /* Initialize board */
    printf("The Cactus - a chess playing AI that is supposed to defeat humans.\n\n"); /* Opening Message */
    render_board(board); /* Display board */

    // Test king move generation
    move_list_t king_moves = {0,0}; /* Initialize an empty move list */
    printf("King Moves:\n");
    generate_king_moves(&king_moves, board); /* Generate king moves */
    for (int i = 0; i < king_moves.count; i++) { /* Loop through all the moves */
        printf("    %d) ", i + 1);
        print_move(king_moves.moves[i]); /* Print move */
    }
    printf("\n");
    return 0;
}
