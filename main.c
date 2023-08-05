/* The Cactus - a chess playing AI that is supposed to defeat humans.
 *  -> Uses Bitboard representation
 *  -> Minmax Search with alpha-beta pruning
 *  -> Iterative deepening method
 *  -> Written in C
*/

#include <stdio.h>
#include <stdlib.h>
#include "bitboards.h"
#include "bitboard_utils.h"

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
    
    // Init board
    board = (Bitboard*)malloc(sizeof(Bitboard)); /* Allocate space for bitboard */
    init_board(board, initial_state, 0); /* Initialize board */
    
    printf("The Cactus - a chess playing AI that is supposed to defeat humans.\n\n"); /* Opening Message */

    render_board(board); /* Display board */

    return 0;
}
