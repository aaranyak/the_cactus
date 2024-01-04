/* Header file for gui_game.c */
#ifndef GUIGAME_H
#define GUIGAME_H
#include "bitboards.h"
#include "moves.h"
int launch_gui(HINSTANCE app_id, int show_cl, Bitboard *board, int human_side, int search_time);

typedef struct GameState {
    /* Stores the state of the game */
    Bitboard *board; /* The current state of the board */
    int perspective; /* The perspective of the human */
    int human_side; /* The side the human is playing */
    int search_time; /* Time given for searching */
    int side; /* So as to differenciate from board->side */
    int mailbox[64]; /* This will be a much easier and efficient lookup scheme */

    // Game details
    move_t last_move;
    int game_over; /* Obviously */
    int draw; /* If it is a draw */
    int white_wins; /* If white wins */
    move_list_t legal_moves; /* List of legal moves (I think this is important */

    // GUI Properties
    HINSTANCE app_id;
    int height;
    int width;

    // Selection Details
    int is_selected; /* If a piece has been selected */
    int selected_piece; /* Selected piece */
    int selected_position; /* On square */
    // Piece hovering
    int is_hovering; /* For drag and drop, if piece is hovering */
    int hover_pos_x; /* Hover position x */
    int hover_pos_y; /* Hover position y */

} GameState;

#endif