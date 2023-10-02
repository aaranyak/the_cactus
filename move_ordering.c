/* move_ordering.c
 * Orders moves using multiple heuristic methods.
 * Sorts them using selection sort
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "lookup_tables.h"
#include "init_magics.h"
#include "make_move.h"
#include "legality_test.h"
#include "generate_moves.h"
#include "evaluation.h"
#include "search.h" /* result_t typedef */
#include "quiescence.h"
#define INF INT_MAX

// Weights for each of the move ordering schemes (Deal with this later).

void sort_moves(move_list_t *move_list, int scores[]);

void order_moves(move_list_t *move_list, Bitboard *board, int use_hash_move, move_t hash_move) {
    /* Order moves to be searched using heuristic methods, so that more branches are likely to be pruned.
     * Ordering methods:
     *  -> Move score from hash table (Not yet implemented).
     *  -> For capture moves, most valuable victim, least valuable attacker.
     *  -> Promoted piece value.
     *  -> Recapured by pawn (penalty).
     *  Think about the following ideas.
     *  -> Killer moves? (not yet implemented)
     *  -> History heuristic? (not yet implemented)
     */

    int move_scores[256]; /* List containing all the move scores */
    // Use following values in the loop
    int score; /* Use in loop */
    int to;
    move_t move; /* Current move */
    int piece;
    int cap_piece;
    int promoted;

    for (int index = 0; index < move_list->count; index++) { /* Loop through all of the moves */
        // Give each move a score.
        move = move_list->moves[index]; /* Get the current move */
        score = 0; /* Reset score */
        piece = (move & MM_PIECE) >> MS_PIECE; /* Get the piece to move */
        to = (move & MM_TO) >> MS_TO; /* Square to move to */
        
        // Check if this is the hash move
        if (use_hash_move) { /* If we have to use the tp table */
            if (move == hash_move) { /* If this is the same as the hash move */
                score = INF; /* Hash Move - Best Move! ;-) */
                move_scores[index] = score; /* Set the move score */
                continue; /* Where were we... */
            }
        }

        if (move & MM_CAP) { /* A Capture move */
            cap_piece = (move & MM_EAT) >> MS_EAT; /* Get the captured piece */
            score += materials[cap_piece] - materials[piece]; /* Most valuable victim, least valuable aggressor */
        }

        if (move & MM_PRO) { /* A promotion move! */
            promoted = (move & MM_PPP) >> MS_PPP; /* Get promoted piece type */
            score += materials[promoted]; /* Give the promoted piece a score */
        }

        if ((1ULL << to) & board->attack_tables[board->side ? pawn_b : pawn_w]) { /* If moving to a square that is attacked by an enemy pawn */
            score -= materials[piece]; /* Subtract the material of the piece, since it will probably be captured on the next move */
        }

        move_scores[index] = score; /* Set the move score */
    }

    sort_moves(move_list, move_scores); /* Sort the moves */
}

void sort_moves(move_list_t *move_list, int scores[]) {
    /* Sorts a list of moves based on the score */
    int swap_value; /* Use in loop */
    int swap_index;
    int index;
    // Perform a selection sort to sort the moves.
    for (swap_index = 0; swap_index < move_list->count; swap_index++) { /* Need not do the last loop */
        for (index = swap_index; index < move_list->count; index++) { /* Loop through the unsorted part of the list */
            if (scores[index] > scores[swap_index]) { /* If this move is better than the first one */
                // Swap the two values
                // Swap the scores
                swap_value = scores[swap_index]; /* Store a temporary value */
                scores[swap_index] = scores[index]; /* The rest should be obvious */
                scores[index] = swap_value;
                // Swap the moves
                swap_value = move_list->moves[swap_index];
                move_list->moves[swap_index] = move_list->moves[index];
                move_list->moves[index] = swap_value;
            }
        }
    }
}
