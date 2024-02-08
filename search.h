/* header file for search.c */
#ifndef SEARCH_H
#define SEARCH_H

#define MAX_THREADS 4

typedef struct search_result {
    /* Search restult */
    int evaluation;
    move_t move;
} result_t;

typedef struct iterative_result {
    /* Return value for iterative deepening */
    int evaluation;
    move_t move;
    int depth;
} id_result_t;

result_t search(Bitboard *board, int depth, int alpha, int beta, int *interrupt_search, int max_time, move_t force_move, int ply, int extensions, int randomize_search, int main_thread);
id_result_t iterative_deepening(Bitboard *board, int search_time);
id_result_t iterative_deepening_interruptable(Bitboard *board, int search_time, int *interrupt_search);
#endif

