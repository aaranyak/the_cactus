/* Test movegen speed */

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

int count_moves(Bitboard *board, int depth); /* Forward declaration */
void do_test(Bitboard *board, int maxdepth); /* Ditto */

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

// Debugging Data
int castling_moves = 0;
int enpas_caps = 0;
int illegals = 0;
int captures = 0;
int promotions = 0;
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
        U64 enpas, castling_rights, key; int ps_eval; /* For unmake move */
        move_t move;
        int local_count = 0;
        for (int i = 0; i < moves.count; i++) { /* Loop through all pseudo-legal moves */
            if (is_legal(board, moves.moves[i])) { /* If this is a legal move */
                move = moves.moves[i];
                make_move(board, moves.moves[i], &enpas, &castling_rights, &key, &ps_eval); /* make the move */
                local_count = count_moves(board, depth - 1); /* The counting is recursive */
                count += local_count;
                unmake_move(board, moves.moves[i], &enpas, &castling_rights, &key, &ps_eval); /* unmake the move */
                if (depth == 1) {
                    if (move & MM_CAP || move & MM_EPC) {
                        captures++;
                    }
                    if (move & MM_CAS) castling_moves++;
                    if (move & MM_EPC) enpas_caps++;
                    if (move & MM_PRO) promotions++;
                } else if (depth == 2) {
                    // print_move(move);
                    // printf(": %d, ", local_count);
                }
            }
        }
        return count;
    } else {
        return 1;
    }
}

