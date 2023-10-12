/* The Cactus
 *  -> Minmax Search with alpha-beta pruning
 *  -> Iterative deepening method
 *  -> Written in C
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "init_magics.h"
#include "make_move.h"
#include "lookup_tables.h"
#include "legality_test.h"
#include "generate_moves.h"
#include "perft_test.h"
#include "search.h"
#include "evaluation.h"
#include "quiescence.h"
#include "queen_moves.h"
#include "zobrist_hash.h"
#include "tp_table.h"
#include "gui_game.h"
#define INF INT_MAX

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
    // Initialize pre-initialized data */
    init_tp_table();
    init_hash_keys();
    init_magic_tables();
    // Initialize the board */
    Bitboard board = {0,0,0,0}; /* Allocate space for bitboard */
    init_board(&board, initial_state, 1);
    // Start a game with the GUI
    int human_side = 1; /* The side of the human to play */
    if (argc - 1) if (strcmp(argv[1], "black\n")) human_side = 0;/* if arguement black, human side is 0 */
    return launch_gui(&board, argc, argv, human_side, 10);
}
