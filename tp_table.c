/* Code for managing the transposition hash table */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "move_gen_utils.h"
#include "lookup_tables.h"
#include "legality_test.h"
#include "tp_table.h"
#include "zobrist_hash.h"

#define TP_SIZE 256 /* TP Table size in megabytes */

int hash_move_used = 0;

shared_entry_t tp_table[(TP_SIZE  * 1000000) / sizeof(shared_entry_t)]; /* Transposition Table Size is set above */
int tp_size = (TP_SIZE  * 1000000) / sizeof(shared_entry_t); /* Set TP Table Size */

void init_tp_table() {
    /* Resets all the values in the TP Table */
    for (int i = 0; i < tp_size; i++) { /* Loop through all the entries in the tp_table */
        tp_table[i] = {0, 0, 0, 0}; /* Corrupted entry, basically (unless by some extreme coincidence the data happens to be same as the key */
    }
}

inline int corrupted_entry(shared_entry_t entry) {
    /* Detects if an entry is corrupted */
    if (entry.key_1 ^ entry.data_1 != entry.key) return 1; /* Entry has been corrupted */
    if (entry.key_2 ^ entry.data_2 != entry.key) return 1; /* Entry has been corrupted */
    return 0; /* Entry has not been corrupted */

int to_replace(int depth, int index) { 
    /* Whether to replace the TP Table entry or not */
    shared_entry_t original = tp_table[index]; /* Retrieve what was already there */
    if (corrupted_entry(entry)) return 1; /* If entry is corrupted, automatically replace */
    int depth = (original.data_2 & EM_DEPTH) >> ES_DEPTH; /* Get the previous depth */
    else if (entry.depth >= depth) return 1;
    else return 0; /* Otherwise don't replace */
}


void add_entry(U64 key, int eval, int depth, int age, move_t best_move, node_t node_type) {
    /* Add an entry to the tp_table */
    int index = key % tp_size; /* You have to get the index of the entry in the tp table */
    if (to_replace(depth, index)) { /* If we should replace */
        /* Set the entry */
        // Initialize
        U64 data_1 = 0;
        U64 data_2 = 0;
        U64 key_1 = 0;
        U64 key_2 = 0;

        // Set data values.
        data_1 |= (eval & EM_EVAL) << ES_EVAL; /* Add the evaluation */
        data_1 |= (best_move & EM_MOVE) << ES_MOVE; /* Add the best move */
        data_2 |= (depth & EM_DEPTH) << ES_DEPTH; /* Add the depth */
        data_2 |= (age & EM_AGE) << ES_AGE; /* Add the move age */
        data_2 |= (node_type & EM_TYPE) << ES_TYPE; /* Add the node type */

        key_1 = key ^ data_1; /* Set the first key (lockless hashing) */
        key_2 = key ^ data_2; /* Set the second key (lockless hashing */
        // Set it in the entry
        tp_table[index].key = key; /* Set the key on the shared entry */
        tp_table[index].key_1 = key_1; /* Set the first key */
        tp_table[index].key_2 = key_2; /* Set the second key */
        tp_table[index].data_1 = data_1; /* Set the first data */
        tp_table[index].data_2 = data_2; /* Set the second data */
    }
}

entry_t get_entry(U64 key) {
    /* Get the entry from the tp table by key */
    int index = key % tp_size; /* get the index of the position */
    shared_entry_t shared_entry = tp_table[index]; /* Get the position */
    if (!corrupted_entry(shared_entry) && shared_entry.key == key) { /* The entry is not corrupted and it is this entry */
        /* Retrieve the values */
        entry_t entry = EMPTY_ENTRY; /* We need to initialize it somehow */
        entry.key = key; /* I don't think it is necessary */
        entry.eval = share
}
