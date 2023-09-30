/* header file for search.c */
#ifndef SEARCH_H
#define SEARCH_H
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

result_t search(Bitboard *board, int depth, int alpha, int beta, int *interrupt_search, int max_time);

id_result_t iterative_deepening(Bitboard *board, int search_time);
#endif

