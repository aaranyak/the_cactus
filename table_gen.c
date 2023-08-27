/* leaper_tables.c
 * For now, generate the lookup tables for the leaping piece moves.
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "lookup_tables.h"
#include "bitboard_utils.h"


int main() {
    printf("static U64 bishop_masks[64] = {");
    Bitboard board = {0,0,0,0};
    for (int i = 0; i < 64; i++) {
        /* loop through all the squares */
        U64 d = bishop_masks[i];
        d &= ~(1ULL << i);
        printf(", 0x%016lx", d);
        board.pieces[pawn_b] = d; 
        //render_board(&board);
    }
    printf("};\n");
    return 0;
}
