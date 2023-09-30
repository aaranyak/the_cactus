/* leaper_tables.c
 * For now, generate the lookup tables for the leaping piece moves.
*/

#include <stdio.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "lookup_tables.h"

int pst[64] = {
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  0,  0,  0,-20,-40,
-30,  0, 10, 15, 15, 10,  0,-30,
-30,  5, 15, 20, 20, 15,  5,-30,
-30,  0, 15, 20, 20, 15,  0,-30,
-30,  5, 10, 15, 15, 10,  5,-30,
-40,-20,  0,  5,  5,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50,
};

int main() {
    Bitboard board = {0,0,0,0};
    U64 pawn_attacks;
    U64 position;
    printf("{");
    for (int i = 0; i < 64; i++) {
        /* loop through all the squares */
        printf("%d, ", pst[i%8 + ((7 - i/8)*8)]);
    }
    printf("}\n");
    return 0;
}
