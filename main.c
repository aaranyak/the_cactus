/* The Cactus
 *  -> Minmax Search with alpha-beta pruning
 *  -> Iterative deepening method
 *  -> Written in C
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
#define INF INT_MAX

void play_game(Bitboard *board, int side) {
    while (1) {
        if (board->side == side) {
            /* Human move */
            move_list_t random = {0,0};
            generate_moves(board, &random);
            for (int i = 0; i < random.count; i++){
                print_move(random.moves[i]);
            }
            char move_title[300];
            printf("Enter move: ");
            fgets(move_title, 300, stdin);
            move_list_t moves = {0,0};
            generate_moves(board, &moves);
            for (int i = 0; i < moves.count; i++) {
                char this_name[300] = {0};
                move_name(moves.moves[i], this_name);
                if (strcmp(move_title, this_name) == 0) {
                    U64 a, b, c;
                    int d;
                    system("clear");
                    printf("The Cactus - a chess AI that is supposed to defeat humans in chess \n\n");
                    make_move(board, moves.moves[i], &a, &b, &c, &d);
                    render_board(board);
                    break;
                }
            }
        } else {
            hash_move_used = 0;
            id_result_t result = iterative_deepening(board, 10); /* Search for 10 seconds */
            U64 a, b, c;
            int d;
            move_t move = result.move;
            system("clear");
            printf("The Cactus - a chess AI that is supposed to defeat humans in chess \n\n");
            make_move(board, move, &a, &b, &c, &d);
            render_board(board);
            printf("Move: "); print_move(move);
            printf("Evaluation: %d\n", -result.evaluation);
            printf("Depth: %d\n", result.depth);
            printf("\n\n");
        }
    }
}
int main(int argc, char **argv) {
    /* Run chess engine */

    // The board state to start with
    char initial_state[64] = {
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ','K','Q',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ','p',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ','n',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ','q','n','r','k',' ',' ',
    }; /* An Array of characters as the starting board state */
    
    // Initialize pre-initialized data */
    init_tp_table();
    init_hash_keys();
    init_magic_tables();
    // Initialize the board */
    Bitboard board = {0,0,0,0}; /* Allocate space for bitboard */
    init_board(&board, initial_state, 0);
    // Render board and print opening message 
    printf("The Cactus - a chess AI that is supposed to defeat humans in chess \n\n");
    render_board(&board); /* Display the board on the screen */
    board.moves = 131;
    board.castling_rights = 0;
    // Test search by playing test game
    play_game(&board, 1);

    
    
    return 0;
}
