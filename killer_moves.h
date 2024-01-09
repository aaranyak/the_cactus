/* Header file for killer_moves.c */
#ifndef KILLER_MOVES_H
#define KILLER_MOVES_H
int is_killer(move_t move, int depth);
void add_killer(move_t move, int depth);
void clear_killers();
#endif
