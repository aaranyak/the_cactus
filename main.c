/* The Cactus
 *  -> Minmax Search with alpha-beta pruning
 *  -> Iterative deepening method
 *  -> Written in C
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "pawn_moves.h"
#include "king_moves.h"
#include "knight_moves.h"
#include "rook_moves.h"
#include "bishop_moves.h"
#include "queen_moves.h"
#include "castling_moves.h"
#include "move_gen_utils.h"
#include "lookup_tables.h"
#include "init_magics.h"
#include "make_move.h"
#include "legality_test.h"
#include "generate_moves.h"

int count_moves(Bitboard *board, int depth); /* Forward declaration */
void do_test(Bitboard *board, int maxdepth); /* Ditto */

// Debugging Data
int castling_moves = 0;
int enpas_caps = 0;
int illegals = 0;
int captures = 0;
int promotions = 0;

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
    
    // Init board
    Bitboard board; /* Allocate space for bitboard */
    
    printf("The Cactus - a chess playing AI that is supposed to defeat humans.\n\n"); /* Opening Message */
    parse_fen(&board, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "); /* Parse the fen string to create the board */
    move_t move = set_move(8, 24, pawn_w, 0);
    move |= MM_DPP;
    U64 a, b, c;
    //make_move(&board, move, &a, &b, &c);
    init_magic_tables(); /* For rook movegen */
    render_board(&board); /* Display the board on the screen */
    do_test(&board, 10); /* Run the PERFT test */
    return 0;
}

void do_test(Bitboard *board, int maxdepth) {
    int depth = 0; /* At each depth, count moves */
    printf("Testing Movegen\n");
    while (depth <= maxdepth) { /* Until depth 4 */
        // Reset debugging data.
        castling_moves = 0;
        enpas_caps = 0;
        captures = 0;
        illegals = 0;
        promotions = 0;
        int move_count = count_moves(board, depth);
        // Print debugging data
        printf("Move count (at depth %d) - %d\n", depth, move_count);
        printf("    Captures - %d, EP Captures - %d, Promotions - %d, Castling - %d\n", captures, enpas_caps, promotions, castling_moves);
        depth++;
    }
    printf("\n\n");
}
int count_moves(Bitboard *board, int depth) {
    /* Counts all moves at a certian depth */
    if (depth) { /* Not reached end of search */
        // Generate all possible moves
        move_list_t moves = {0,0}; /* Pseudo-legal move list */
        generate_moves(board, &moves);
        // Do the recursive loop
        int count = 0;
        U64 enpas, castling_rights, key; /* For unmake move */
        move_t move;
        int local_count = 0;
        for (int i = 0; i < moves.count; i++) { /* Loop through all pseudo-legal moves */
            if (is_legal(board, moves.moves[i])) { /* If this is a legal move */
                move = moves.moves[i];
                make_move(board, moves.moves[i], &enpas, &castling_rights, &key); /* make the move */
                local_count = count_moves(board, depth - 1); /* The counting is recursive */
                count += local_count;
                unmake_move(board, moves.moves[i], &enpas, &castling_rights, &key); /* unmake the move */
                if (depth == 1) {
                    if (move & MM_CAP || move & MM_EPC) {
                        captures++;
                    }
                    if (move & MM_CAS) castling_moves++;
                    if (move & MM_EPC) enpas_caps++;
                    if (move & MM_PRO) promotions++;
                } else if (depth == 3) {
                    //print_move(move);
                    //printf(": %d, ", local_count);
                }
            }
        }
        return count;
    } else {
        return 1;
    }
}
