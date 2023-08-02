/* The Cactus - a chess playing AI that is supposed to defeat humans.
 *  -> Uses Bitboard representation
 *  -> Minmax Search with alpha-beta pruning
 *  -> Iterative deepening method
 *  -> Written in C
*/

#include <stdio.h>
#include <stdlib.h>
#include "bitboards.h"
// #include "bitboard_utils.h"

int main(int argc, char **argv) {
    /* Run chess engine */

    // The board state to start with
    char initial_state[64] = {
        "rnbqkbnr"
        "pppppppp"
        "        "
        "        "
        "        "
        "        "
        "PPPPPPPP"
        "RNBQKBNR"
    }; /* An Array of characters as the starting board state */

    printf("The Cactus - a chess playing AI that is supposed to defeat humans.\n"); /* Opening Message */
    
    return 0;
}
