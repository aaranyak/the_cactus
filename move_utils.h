/* header file for move_utils.c */
#ifndef MOVEUTILS_H /* Header Guard */
#define MOVEUTILS_H
move_t set_move(
        uint8_t from,
        uint8_t to,
        uint8_t piece,
        uint8_t captured
        );
void add_move_to_list(move_list_t *list, move_t move);
void print_move(move_t move);
void move_name(move_t move, char *enter);
#endif
