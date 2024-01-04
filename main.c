/*
    The Cactus - A chess engine that is supposed to defeat humans in chess.
    Copyright (C) 2023  Aaranyak Ghosh

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* The Cactus
 *  -> Minmax Search with alpha-beta pruning
 *  -> Iterative deepening method
 *  -> Written in C
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "gui_game.h"
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
#define INF INT_MAX

int WINAPI WinMain(HINSTANCE app_id, HINSTANCE previous_id, LPSTR command_line, int show_cl) { /* The main function in using the Windows API is structured slightly differently  */
    /* Run the chess engine */

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
    int human_side = 0;
    int message_id = MessageBox(
            NULL,
            "Would you like to play as black or whiite\nClick YES for white, and NO for black.",
            "Black or white",
            MB_ICONQUESTION | MB_YESNO
        );
    if (message_id == IDYES) human_side = 1;
    return launch_gui(app_id, show_cl, &board, human_side, 10); /* Launch GUI */
}
