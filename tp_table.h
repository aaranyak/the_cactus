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

typedef enum node_t {node_pv = 0, node_cut = 1, node_all = 2} node_t;

typedef struct entry_t {
    U64 key; /* The zobrist key */
    int eval; /* The evaluation */
    int depth; /* Depth at which this was searched */
    int age; /* The number of moves at which this position was played */
    move_t best_move; /* The best move from this position */
    node_t node_type; /* Whether this has been pruned or not */
} entry_t;

typedef struct shared_entry_t {
    /* This is the hash table entry that will be stored in the actual table */
    U64 key; /* This is the actual key (To check for a corrupted entry, while replacing */
    U64 key_1; /* This is the key xor'd with the first data segment */
    U64 key_2; /* This will be the key xor'd with the second data segment */
    U64 data_1; /* First data segment - contains evaluation and best move respectively */
    U64 data_2; /* Second data segment - Contains depth, age and node type respectively */
} shared_entry_t; /* The type */


// Shared entry shifts and masks
// Data 1
#define EM_EVAL 0x00000000ffffffff /* Entry data mask for evaluation */
#define EM_MOVE 0xffffffff00000000 /* Entry data mask for node type */
// Data 2
#define EM_DEPTH 0x00000000ffffffff /* Entry data mask for depth */
#define EM_AGE 0x0fffffff00000000 /* Entry data mask for age */
#define EM_TYPE 0xf000000000000000 /* Entry data mask for node type */
// Shifts
#define ES_EVAL 0 /* Bitshift for eval */
#define ES_MOVE 32 /* Ditto for move */
#define ES_DEPTH 0 /* Depth */
#define ES_AGE 32 /* Position Age */
#define ES_TYPE 56 /* Node type */


#define invalid_entry(e) (e.depth == -1)
#define EMPTY_ENTRY (entry_t){0,0,-1,0,0,0}
int corrupted_entry(shared_entry_t entry);
void add_entry(U64 key, int eval, int depth, int age, move_t best_move, node_t node_type);
entry_t get_entry(U64 key);
void init_tp_table();
extern int tp_size;
extern shared_entry_t tp_table[]; /* Transposition Table */
extern int hash_move_used;
#endif
