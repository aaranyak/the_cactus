/* killer_moves.c
 * Implements the killer heuristic in the move ordering scheme
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

#define MAX_DEPTH 20

move_t killers[MAX_DEPTH][2] = {0}; /* Contains killer moves */
int history_heuristic[64][64] = {0}; /* Contains history heuristic */


int is_killer(move_t move, int depth) {
    /* Check if move is a killer move */
    if (depth >= MAX_DEPTH) return 0;
    if (move == killers[depth][0]) return 1;
    if (move == killers[depth][1]) return 1;
    return 0;
}

void add_killer(move_t move, int depth) {
    /* Adds a killer move to the killers list */
    if (depth >= MAX_DEPTH) return; /* Cannot be greater than max depth */
    if (!is_killer(move, depth)) {
        killers[depth][1] = killers[depth][0]; /* Shift last killer */
        killers[depth][0] = move; /* Add the killer move */
    }
}

void clear_killers() {
    /* Clears the killer moves list */
    for (int i = 0; i < 20; i++) {
        killers[i][0] = 0;
        killers[i][1] = 0;
    }
}

// History Heuristic

void clear_history() {
    /* Clear the History Heuristic */
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            history_heuristic[x][y] = 0;
        }
    }
}

int get_history(move_t move) {
    /* Get the history bias */
    return history_heuristic[(move & MM_FROM) >> MS_FROM][(move & MM_TO) >> MS_TO]; /* Get the history heuristic */
}

void set_history(move_t move, int depth) {
    /* Set the history heuristic */
    history_heuristic[(move & MM_FROM) >> MS_FROM][(move & MM_TO) >> MS_TO] = depth*depth; /* Get the history heuristic */
}
