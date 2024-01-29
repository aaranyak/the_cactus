/* Header file for gui_game.c */

#ifndef GUIGAME_H
#define GUIGAME_H
#include <gtk/gtk.h>
typedef struct GameState {
    // Basics
    Bitboard *board; /* The current board state */
    int human_side; /* The side that the human is playing */
    int side; /* So as not to confuse with the board->side */
    int game_over; /* Until game is not over */
    int draw; /* If it is a draw */
    int white_wins; /* If white wins */
    int think_time; /* AI Think Time */ 
    // Widgets to update
    GtkWidget *drawing_area; /* Drawingarea to update */
    GtkWidget *eval_text; /* Evaluation Text */
    GtkWidget *move_text; /* Last Move Text */
    GtkWidget *depth_text; /* Depth Searched text */
    GtkWidget *side_text; /* Side To Move text */
    GtkWidget *think_text; /* Shows that the engine is thinking */
    // Visuals and data
    int evaluation; /* AI move evaluation */
    int depth; /* Depth searched */
    move_t last_move; /* Last move played */  
    int mailbox[64]; /* A mailbox notation of the board for easy rendering */
    int perspective; /* Which side to view the board from */
    move_list_t legal_moves; /* List of legal moves (I think this is important */

    // Selected piece
    int is_selected; /* If a piece has been selected */
    int selected_piece; /* Selected piece */
    int selected_position; /* On square */
    // Piece hovering
    int is_hovering; /* For drag and drop, if piece is hovering */
    int hover_pos_x; /* Hover position x */
    int hover_pos_y; /* Hover position y */

    // Logging
    char *log_filename; /* Log File Pointer */

} GameState;
int launch_gui(Bitboard *board, int argc, char **argv, int human_side, int search_time, char *log_filename);
int launch_options(Bitboard *board, int argc, char **argv);
move_list_t generate_legal_moves(Bitboard *board);
#endif
