/* leaper_tables.c
 * For now, generate the lookup tables for the leaping piece moves.
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "lookup_tables.h"

int main() {
    printf("U64 pawn_attacks_b[64] = {");
    for (int i = 0; i < 64; i++) {
        /* loop through all the squares */
        U64 position = (U64)1 << i;
        U64 times = 0;
        times |= (position >> 7) | (position >> 9);
        times &= ~(files[0] & files[7]);  
        printf(", 0x%016lx", times); 
    }
    printf("};\n");
    return 0;
}
