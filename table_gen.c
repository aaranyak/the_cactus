/* leaper_tables.c
 * For now, generate the lookup tables for the leaping piece moves.
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "lookup_tables.h"
#include "bitboard_utils.h"


int main() {
    printf("U64 king_attacks[64] = {");
    Bitboard board = {0,0,0,0};
    for (int i = 0; i < 64; i++) {
        /* loop through all the squares */
        U64 position = 1ULL << i; /* Current position */
        U64 move_set =
            (position << 8) |
            (position >> 8) |
            ((position << 7) & ~files[7]) |
            ((position >> 1) & ~files[7]) |
            ((position >> 9) & ~files[7]) |
            ((position << 9) & ~files[0]) |
            ((position << 1) & ~files[0]) |
            ((position >> 7) & ~files[0]);
        printf(", 0x%016lx", move_set);
        board.pieces[0] = move_set;
        // render_board(&board);
    }
    printf("};\n");
    return 0;
}
