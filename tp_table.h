/* Header file for tp_table.c
 * Also contains struct for tp table entry
*/

#ifndef TPTABLE_H
#define TPTABLE_H 

// A short note on these node types, forgive me for being a little technical here.
/* Due to the nature of the alpha-beta pruning algorithm, the nodes searched in the search tree may not result in the evaluation being exact.
 * PV-Nodes - These are the nodes in which all the children have been searched, therefore the evaluation is exact, and not a bound.
 * Cut-Nodes - This is a node where the local alpha had exceeded the beta, and therfore the node was pruned, and the value return was a lower bound of the actual evaluation.
 * All-Nodes - These are rather hard to understad, but due to the alpha-beta search, if none of the moves searched exceeds the alpha, the final evaluation will be an upper bound rather than the true evaluation, and will probably be pruned off the search tree.
*/

typedef enum node_t {node_pv, node_cut, node_all} node_t;

typedef struct entry_t {
    U64 key; /* The zobrist key */
    int eval; /* The evaluation */
    int depth; /* Depth at which this was searched */
    int age; /* The number of moves at which this position was played */
    move_t best_move; /* The best move from this position */
    node_t node_type; /* Whether this has been pruned or not */
} entry_t;

#define invalid_entry(e) (e.depth == -1)
#define EMPTY_ENTRY (entry_t){0,0,-1,0,0,0}
void add_entry(U64 key, int eval, int depth, int age, move_t best_move, node_t node_type);
entry_t get_entry(U64 key);
void init_tp_table();
extern int tp_size;
extern entry_t tp_table[]; /* Transposition Table */
extern int hash_move_used;
#endif