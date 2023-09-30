/* Code for managing the transposition hash table */

#include <stdio.h>
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

entry_t tp_table[(TP_SIZE  * 1000000) / sizeof(entry_t)]; /* Transposition Table Size is set above */
int tp_size = (TP_SIZE  * 1000000) / sizeof(entry_t); /* Set TP Table Size */

void init_tp_table() {
    /* Resets all the values in the TP Table */
    for (int i = 0; i < tp_size; i++) { /* Loop through all the entries in the tp_table */
        entry_t entry = {0,0,0,0,0,0};
        entry.depth = -1; /* Invalid entry */
        tp_table[i] = entry; /* Set the entry */
    }
}

int to_replace(entry_t entry, int index) { 
    /* Whether to replace the TP Table entry or not */
    entry_t original = tp_table[index]; /* Get the original entry */
    if (invalid_entry(original)) { /* If there is no entry in the original entry */
        return 1; /* Replace it */
    } else {
        if (entry.depth >= original.depth) return 1; /* If the entry has a greater depth than the original */
        else return 0;
    }
}


void add_entry(U64 key, int eval, int depth, int age, move_t best_move, node_t node_type) {
    /* Add an entry to the tp_table */
    int index = key % tp_size; /* Calculate the index of the entry in the transposition table */
    entry_t entry = {key, eval, depth, age, best_move, node_type}; /* Set the entry object */
    if (to_replace(entry, index)) { /* If it is ok to replace the entry */
        tp_table[index] = entry; /* Set the entry in the table */
    }
}

entry_t get_entry(U64 key) {
    /* Get the entry from the tp table by key */
    int index = key % tp_size; /* Calculate the entry index in the tp table */
    entry_t entry = tp_table[index]; /* Get the entry from the tp_table */
    if (entry.key != key) /* Entry does not match the key */ return EMPTY_ENTRY; /* Return invalid */
    return entry; /* Otherwise, return the entry */
}
