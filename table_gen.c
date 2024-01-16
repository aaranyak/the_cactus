/* leaper_tables.c
 * For now, generate the lookup tables for the leaping piece moves.
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "lookup_tables.h"

int pawn_tables[8] = {0, 0, 5, 7, 10, 25, 40, 0};

int main() {
    printf("endgame_pawn_eval[64] = {");
    for (int i = 0; i < 64; i++)
        printf(", %d", pawn_tables[i/8]);
    
    printf("};\n");
    return 0;
}
