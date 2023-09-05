/* leaper_tables.c
 * For now, generate the lookup tables for the leaping piece moves.
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "lookup_tables.h"


int main() {
    printf("static U64 pawn_attacks_w[64] = {");
    Bitboard board = {0,0,0,0};
    U64 pawn_attacks;
    U64 position;
    for (int i = 0; i < 64; i++) {
        /* loop through all the squares */
        position = 1ULL << i;
        pawn_attacks = ((position >> 7) & ~files[0]) | ((position >> 9) & ~files[7]);
        board.pieces[pawn_w] = pawn_attacks;
        printf(", 0x%016lx", pawn_attacks);
    }
    printf("};\n");
    return 0;
}
