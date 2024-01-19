/* gui_game.c 
 *  -> The gameplay mechanism with GUI
*/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "init_magics.h"
#include "make_move.h"
#include "lookup_tables.h"
#include "legality_test.h"
#include "generate_moves.h"
#include "perft_test.h"
#include "search.h"
#include "evaluation.h"
#include "quiescence.h"
#include "queen_moves.h"
#include "zobrist_hash.h"
#include "tp_table.h"
#include "gui_game.h"
#include "move_gen_utils.h"
#define INF INT_MAX

#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif

char *piece_icons[12] = {"♜", "♞", "♝", "♛", "♚", "♟", "♜", "♞", "♝", "♛", "♚", "♟"}; /* Chess piece icons */

move_list_t generate_legal_moves(Bitboard *board) {
    /* Returns a list of legal moves */
    move_list_t pseudo_legal = {0,0}; /* Pseudo legal moves */
    move_list_t legal = {0,0}; /* Legal moves */
    generate_moves(board, &pseudo_legal); /* generate pseudo-legal moves */
    // Filter out illegal moves
    for (int i = 0; i < pseudo_legal.count; i++) { /* Loop through all legal moves */
        if (is_legal(board, pseudo_legal.moves[i])) { /* If the move is legal */
            add_move_to_list(&legal, pseudo_legal.moves[i]); /* Add it to the legal moves list */
        }
    }
    return legal;
}

void update_game_state(GameState *state, int evaluation, move_t last_move, int depth, int update_text) { 
    /* Update the game state based on the board */
    state->evaluation = evaluation;
    state->last_move = last_move;
    state->depth = depth;
    // Reset selection
    state->is_selected = 0;
    state->is_hovering = 0;
    state->selected_piece = 0;
    state->selected_position = 0;
    state->hover_pos_x = 0;
    state->hover_pos_y = 0;

    
    // Update mailbox notation
    for (int square = 0; square < 64; square++) { /* Loop through all the squares */
        U64 position = 1ULL << square; /* Get position */
        int piece = -1;
        for (int bb = 0; bb < 12; bb++) { /* Loop through all bitboards */
            if (state->board->pieces[bb] & position) { /* If there is a piece on that position */
                piece = bb; /* Set the piece id */
            }
        }
        state->mailbox[square] = piece;
    }

    // Update text
    state->side = state->board->side; /* So that it does not keep changing during search */
    if (update_text) {
        char depth_val[100];
        char eval_val[100];
        char move_val[100];
        char move_name_itself[100];
        sprintf(depth_val, "Search Depth  -  %d", depth);
        sprintf(eval_val, "Evaluation  -  %d", evaluation);
        move_name(last_move, move_name_itself); /* Get name of move */
        sprintf(move_val, "Last Move  -  %s", move_name_itself);
        gtk_label_set_text(GTK_LABEL(state->depth_text), depth_val);
        gtk_label_set_text(GTK_LABEL(state->eval_text), eval_val);
        gtk_label_set_text(GTK_LABEL(state->move_text), move_val);
        gtk_label_set_text(GTK_LABEL(state->side_text), state->side ? "Side To Move - White" : "Side To Move - Black");
    }
    state->legal_moves = generate_legal_moves(state->board); /* Generate legal moves for checking later */
}

gboolean draw_board(GtkWidget *drawing_area, cairo_t *canvas, GameState *state) {
    /* Callback for the drawing of the board */
    int board_size = min(gtk_widget_get_allocated_height(drawing_area), gtk_widget_get_allocated_width(drawing_area)); /* Size of the board */
    float square_size = board_size / 8.0f; /* Size of each square on the board */
    int file; /* Use for file/rank looping */
    int rank; /* Ditto */
    int index; /* Use for direct looping */
    int piece; /* The piece type */

    // Pango for rendering text
    PangoLayout *text; /* The text layout object */
    int char_width; /* Pango width of character */
    int char_height; /* Pango height of character */
    PangoFontDescription *font; /* The font object */
    text = pango_cairo_create_layout(canvas); /* Create the text painter object */
    font = pango_font_description_from_string("Liberation Sans Normal 27"); /* Create a font */
    pango_font_description_set_absolute_size(font, square_size * PANGO_SCALE); /* Set the size of the font to the size of the square (Optimize later) */
    pango_font_description_set_family(font, "Times New Roman");
    pango_layout_set_font_description(text, font); /* Set the font of the text */
    pango_font_description_free(font); /* Free the font data */
    pango_layout_set_alignment(text, PANGO_ALIGN_CENTER); /* Set alignment of character to center */
    
    // Previous move stuff
    int last_move_from = (state->last_move & MM_FROM) >> MS_FROM; /* From square of last move */
    int last_move_to = (state->last_move & MM_TO) >> MS_TO; /* To square of last move */
    // Draw the squares on the board.
    for (rank = 0; rank < 8; rank++) { /* Loop over every row */
        for (file = 0; file < 8; file++) { /* Loop over every square */     
            index = state->perspective ? file + ((7 - rank) * 8) : (7 - file) + (rank * 8); /* Get the square index */
            if (index == last_move_to) cairo_set_source_rgb(canvas, 0.4196, 0.7882, 0.3647); /* Bright green for to square */
            else if (index == last_move_from) cairo_set_source_rgb(canvas, 0.2157, 0.4314, 0.1843); /* Darker green for from square */
            else if ((file + rank) % 2) cairo_set_source_rgb(canvas, 0.4336, 0.5586, 0.4453); /* Black colour */
            else cairo_set_source_rgb(canvas, 0.6758, 0.7383, 0.5586); /* White colour */
            
            cairo_rectangle(canvas, /* Draw a rectangle */
                            file * square_size, rank * square_size, /* X pos, Y pos. */
                            square_size, square_size /* Width, Height */
                            ); /* Draw the square */
            cairo_fill(canvas);
    
            // Draw text
            if (state->mailbox[index] != -1) {
                /* If the square is not empty, draw text */
                piece = state->mailbox[index];
                if (!(state->is_hovering && state->is_selected && state->selected_piece == piece && state->selected_position == index && state->side == state->human_side)) { /* Non-hovering piece */
                    cairo_save(canvas); /* Saves the current canvas layout */
                    pango_layout_set_text(text, piece_icons[piece], -1); /* Set the text of the layout to the correct piece type */
                    if (piece < 6) cairo_set_source_rgb(canvas, 1.0, 1.0, 1.0); /* White piece */
                    else cairo_set_source_rgb(canvas, 0.0, 0.0, 0.0); /* Black piece */
                    pango_cairo_update_layout(canvas, text); /* I think this just connects the layout to the canvas */
                    pango_layout_get_pixel_size(text, &char_width, &char_height); /* Get size of text */
                    cairo_move_to(canvas, (file * square_size) + ((square_size - char_width) / 2), (rank * square_size) + ((square_size - char_height) / 2)); /* Move text to square */
                    pango_cairo_show_layout(canvas, text); /* Draw text on canvas */
                    cairo_restore(canvas); /* Restore the saved work (I think) */
                }
            }
        }
    }
    
    // Deal with target squares.
    if (state->is_selected && state->side == state->human_side) { /* If there is a selected piece */
        // Display all the possible squares it can move to */
        int move;
        int to;
        for (index = 0; index < state->legal_moves.count; index++) { /* Loop through all the legal moves */
            move = state->legal_moves.moves[index];
            if (move & MM_CAS) { /* If this is a castling move */
                if (state->selected_piece == king_w  || state->selected_piece == king_b) {
                    // Calculate the castling index */
                    if (state->side) {
                        if (move & MM_CSD) to = 2;
                        else to = 6;
                    } else {
                        if (move & MM_CSD) to = 58;
                        else to = 62;
                    }
                    if (state->perspective) {
                        file = to % 8; /* Get file of to-square */
                        rank = 7 - (to / 8); /* Get rank of to square */
                    } else {
                        file = 7 - (to % 8); /* Get file of to-square */
                        rank = to / 8; /* Get rank of to square */
                    }
                    // Draw circle
                    cairo_set_source_rgb(canvas, 0.0353, 0.3382, 0.0118); /* Set the colour to a good green */
                    cairo_arc(canvas, (file * square_size) + (square_size / 2), (rank * square_size) + (square_size / 2), square_size / 8.0f,0, M_PI * 2); /* Draw a circle at that point */
                    cairo_fill(canvas); /* Actually draw the circle */
                }
            }
            else if (((move & MM_PIECE) >> MS_PIECE) == state->selected_piece && ((move & MM_FROM) >> MS_FROM) == state->selected_position) { /* If this move is of the selected piece */
                to = (move & MM_TO) >> MS_TO; /* Get the to-square */
                // calculate file & rank.
                if (state->perspective) {
                    file = to % 8; /* Get file of to-square */
                    rank = 7 - (to / 8); /* Get rank of to square */
                } else {
                    file = 7 - (to % 8); /* Get file of to-square */
                    rank = to / 8; /* Get rank of to square */
                }
                // Draw circle
                cairo_set_source_rgb(canvas, 0.0353, 0.3382, 0.0118); /* Set the colour to a good green */
                cairo_arc(canvas, (file * square_size) + (square_size / 2), (rank * square_size) + (square_size / 2), square_size / 8.0f,0, M_PI * 2); /* Draw a circle at that point */
                cairo_fill(canvas); /* Actually draw the circle */
            }
        }
    }
    
    // Deal with hovering piece
    if (state->is_hovering && state->side == state->human_side) { /* There is a piece that is currently hovering */
        piece = state->selected_piece; /* Hovering piece */
        cairo_save(canvas); /* Saves the current canvas layout */
        pango_layout_set_text(text, piece_icons[piece], -1); /* Set the text of the layout to the correct piece type */
        if (piece < 6) cairo_set_source_rgb(canvas, 1.0, 1.0, 1.0); /* White piece */
        else cairo_set_source_rgb(canvas, 0.0, 0.0, 0.0); /* Black piece */
        pango_cairo_update_layout(canvas, text); /* I think this just connects the layout to the canvas */
        pango_layout_get_pixel_size(text, &char_width, &char_height); /* Get size of text */
        cairo_move_to(canvas, state->hover_pos_x - (char_width / 2), state->hover_pos_y - (char_height / 2)); /* Move text to square */
        pango_cairo_show_layout(canvas, text); /* Draw text on canvas */
        cairo_restore(canvas); /* Restore the saved work (I think) */
    }
    if (state->game_over) { /* If the game is over */
        // Display text with game over state
        font = pango_font_description_from_string("Ubuntu Bold 35"); /* Create a font */
        pango_layout_set_font_description(text, font); /* Set the font of the text */
        pango_font_description_free(font); /* Free the font */
        pango_layout_set_alignment(text, PANGO_ALIGN_CENTER); /* Set alignment of character to center */
        pango_layout_set_text(text, state->draw ? "Ends in Draw" : state->white_wins ? "White Checkmates Black!" : "Black Checkmates White!", -1); /* Set text based on game end state */
        cairo_save(canvas); /* Saves the current canvas layout */
        cairo_set_source_rgb(canvas, 0, 0, 0); /* Grey for now */
        pango_cairo_update_layout(canvas, text); /* I think this just connects the layout to the canvas */
        pango_layout_get_pixel_size(text, &char_width, &char_height); /* Get size of text */
        cairo_move_to(canvas, (board_size / 2) - (char_width / 2), (board_size / 2) - (char_height / 2)); /* Move text to square */
        pango_cairo_show_layout(canvas, text); /* Paint layout on canvas */
        cairo_restore(canvas); /* Restore saved state */

    }    
    g_object_unref(text); /* Delete the text painter object */
}

void flip_board_callback(GtkButton *button, GameState *state) {
    /* When the flip board button is clicked */
    state->perspective = !state->perspective; /* Flip!! */
    gtk_widget_queue_draw(state->drawing_area); /* Update drawing area */
}

void mouse_move_callback(GtkWidget *drawing_area, GdkEventMotion *event, GameState *state) {
    /* Update hover condition when mouse moves */
    state->hover_pos_x = event->x; /* Update mouse position x */
    state->hover_pos_y = event->y; /* Update mouse position y */
    gtk_widget_queue_draw(state->drawing_area); /* Update drawing area */
} 

void mouse_down_callback(GtkWidget *drawing_area, GdkEventButton *event, GameState *state) { 
    /* Mouse button down event */
    if (!state->game_over && event->button == 1) { /* If the game is not yet over left mouse button is pressed */
        if (state->side == state->human_side) { /* If human to move */
            // Handle mouse down for selection & Drag n drop
            int board_size = min(gtk_widget_get_allocated_height(drawing_area), gtk_widget_get_allocated_width(drawing_area)); /* Size of the board */
            float square_size = board_size / 8.0f; /* Size of each square on the board */
            int file = state->hover_pos_x / square_size; /* Get the file that the mouse is on */
            int rank = state->hover_pos_y / square_size; /* Get the rank that the mouse is on */
            int index = state->perspective ? file + ((7 - rank) * 8) : (7 - file) + (rank * 8); /* Get the square index */
            int piece = state->mailbox[index];
            if (piece == -1 || !(state->human_side ? piece < 6 : piece > 5)) { /* Clicked on empty square */
                // Clear Selection
                state->is_selected = 0;
                state->selected_piece = 0;
                state->selected_position = 0;
                state->is_hovering = 0;
            } else {
                // Something has been selected
                state->is_selected = 1; /* Something has been selected */
                state->selected_piece = piece; /* Set selected piece */
                state->selected_position = index; /* On square */
                state->is_hovering = 1; /* Start hovering for Drag n Drop */
            }
            gtk_widget_queue_draw(state->drawing_area); /* Update drawing area */
        }
    }
}

void update_move_log(GameState *state, int count, int side, U64 key, move_t move, int depth, int evaluation) { /* Log a move in the log file */
    /* Updates the move on the log file */
    FILE *log = fopen(state->log_filename, "a+"); /* Open file in append mode */
    char move_as_str[300] = {0}; /* Put move here */
    move_name(move, move_as_str); /* Get Move Name */
    fprintf(log, "%d, %s, %016lx, %08x, \"%s\", %d, %d\n", count, side ? "White" : "Black", key, move, move_as_str, depth, evaluation); /* Print move metadata */
    fclose(log); /* Close File */

}

void play_move_on_board(GameState *state, move_t move, int eval, int depth) {
    /* Play a move on the board, and update status */
    if (!state->game_over) {
        U64 a, b, c; int d; /* Saved data stuff for move */
        if (state->log_filename) update_move_log(state, state->board->moves + 1, state->side, state->board->key, move, depth, eval); /* Log the move */
        make_move(state->board, move, &a, &b, &c, &d); /* Make the move on the board */
        update_game_state(state, eval, move, depth, 1); /* Update the game state with the last move */
        // Check for checkmate
        if (!state->legal_moves.count) { /* Check if there are no legal moves left */
            if (is_check(state->board, state->side)) { /* If king is in check */
                state->game_over = 1; /* Game Over (Checkmate) */
                // Update game-over state
                if (!state->side) state->white_wins = 1;
                else state->white_wins = 0;
                state->draw = 0;
            } else {
                state->draw = 1; /* Draw */
                state->game_over = 1; /* Game Over (Stalemate */
            }
        }
        // Detect a draw (only kings left).
        U64 piece_boards = colour_mask(state->board, 0) | colour_mask(state->board, 1); /* Get the total board */
        if (popcount(piece_boards) < 3) { /* If only two kings are left */
            state->game_over = 1; /* Return a draw (only two kings left) */
            state->draw = 1; /* Draw by kings left */
        }

        // Detect draw by repetitions
        if (get_repetitions(state->board) >= 3) { /* If there are more than two repetitions */
            state->game_over = 1; /* Return a draw (only two kings left) */
            state->draw = 1; /* Draw by kings left */
        }
    }
}

void *engine_think(void *state_pointer) {
    /* Function to make AI start thinking */
    id_result_t result; /* Search result */
    GameState *state = (GameState*)state_pointer; /* Cast the state pointer into a gamestate */
    gtk_label_set_markup(GTK_LABEL(state->think_text), g_markup_printf_escaped("<span size=\"large\" style=\"italic\">%s</span>", "The Cactus is Thinking"));
    result = iterative_deepening(state->board, state->think_time);
    play_move_on_board(state, result.move, result.evaluation, result.depth); /* Play the move on the board, and update values */
    gtk_label_set_markup(GTK_LABEL(state->think_text), g_markup_printf_escaped("<span size=\"large\" style=\"italic\">%s</span>", ""));
    gtk_widget_queue_draw(state->drawing_area); /* Update drawing area */

}

void launch_engine_thread(GameState *state) {
    /* Launches the thinking thread of the ai */
    if (!state->game_over) { /* If game not over */
        pthread_t think_thread; /* Launch a new thread for the engine to think */
        int retval; /* Returned value */

        retval = pthread_create(&think_thread, NULL /* We need to figure out exactly what this parameter does */, engine_think, state); /* Launch a new thread for the engine thinking so that it does not disturb the gui */
    }
}

void play_human_move(GameState *state, int index) {
    /* Search for a move and if it is there play it for a human */
    move_t is_move = 0; /* Check if this is a legal move */
    move_t move;
    int i;
    int from;
    int to;
    int move_piece;
    for (i = 0; i < state->legal_moves.count; i++) { /* Loop through all the moves */
        move = state->legal_moves.moves[i]; /* Get the move */
        from = (move & MM_FROM) >> MS_FROM;
        to = (move & MM_TO) >> MS_TO;
        move_piece = (move & MM_PIECE) >> MS_PIECE;
        // Also handle castling moves.
        if (move & MM_CAS) { /* If this is a castling move */
            move_piece = state->side ? king_w : king_b; /* Castling is done by the king */
            from = state->side ? 4 : 60; /* Start square has to be king square */
            int cas_side = move & MM_CSD;
            to = state->side ? cas_side ? 2 : 6 : cas_side ? 58 : 62; /* Set the castling side */
        }
        if (from == state->selected_position && to == index && move_piece == state->selected_piece) { /* If this is a matching move */
            if (move & MM_PRO) { /* If this is a pawn promotion move, check which piece to promote to */
                GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(state->drawing_area)); /* Get the top level window */
                GtkWidget *popup_dialog = gtk_dialog_new_with_buttons( /* Create an interactive dialog with buttons */
                    "Promote To - ", /* Dialog title */
                    GTK_WINDOW (window), /* Toplevel window */
                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, /* Flags */
                    /* Here come the buttons */
                    "_Queen"/* Button title */, 3 /* Response id */, "Bishop", 2, "Knight", 1, "Rook", 0, /* Buttons for each piece to promote to */
                    "_Cancel", /* Ditto */
                    -1,
                    NULL /* List ends with null */
                    );
                gint response = gtk_dialog_run(GTK_DIALOG (popup_dialog)); /* Open the dialog */
                gtk_widget_destroy(popup_dialog); /* Destroy the dialog */
                if (response == -1) return;
                else { /* Set the move to piece type */
                    move &= ~MM_PPP; /* Clear the current one */
                    move |= (response << MS_PPP); /* Add the new promoted piece type */
                }
            }
            is_move = move;
            break;
        }
    } if (is_move) { /* A move has been found */
        play_move_on_board(state, is_move, 0, 0);
        launch_engine_thread(state); /* Let the AI start thinking */
    }
}

void mouse_up_callback(GtkWidget *drawing_area, GdkEventButton *event, GameState *state) {
    /* Mouse up event */
    if (!state->game_over && event->button == 1) { /* If the game is not yet over left mouse button is pressed */
        if (state->side == state->human_side) { /* If human to move */
            if (state->is_selected) { /* If a piece is actually selected */
                // For now just stop hovering, we can deal with all that stuff later
                int board_size = min(gtk_widget_get_allocated_height(drawing_area), gtk_widget_get_allocated_width(drawing_area)); /* Size of the board */
                float square_size = board_size / 8.0f; /* Size of each square on the board */
                int file = state->hover_pos_x / square_size; /* Get the file that the mouse is on */
                int rank = state->hover_pos_y / square_size; /* Get the rank that the mouse is on */
                int index = state->perspective ? file + ((7 - rank) * 8) : (7 - file) + (rank * 8); /* Get the square index */
                int piece = state->mailbox[index];
                state->is_hovering = 0; /* Stop hovering */
                
                // Check if this is a legal move, if so make it on the board */
                play_human_move(state, index); /* Play the move */
                gtk_widget_queue_draw(state->drawing_area); /* Update drawing area */
            }
        }
    }
}


void create_game_window(GtkApplication **app, GtkWidget **game_window, GtkWidget **board_canvas, GameState *state) {
    /* Create the window for the game being played */
    // Declare Widgets
    GtkWidget *container;
    GtkWidget *side_container;
    GtkWidget *flip_button;
    GtkWidget *button_box; /* Styling and sizing */
    // Texts
    GtkWidget *depth_text; /* Depth Searched Text */
    GtkWidget *eval_text; /* Evaluation Text */
    GtkWidget *move_text; /* Last Move Text */
    GtkWidget *side_text; /* Side To Move Text */
    GtkWidget *think_text; /* Shows that engine is thinking */

    // Text Boxes
    GtkWidget *depth_box; /* Depth Searched Box */
    GtkWidget *eval_box; /* etc. */
    GtkWidget *move_box;
    GtkWidget *side_box;
    GtkWidget *think_box;

    // Create Widgets
    *game_window = gtk_application_window_new(*app); /* Create a new gtk window */
    *board_canvas = gtk_drawing_area_new(); /* Create a canvas to draw the board on */
    state->drawing_area = *board_canvas; /* Set the drawing area on the game state */
    container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* Create a row box */
    side_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); /* Create the side info box */
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* To style and size the button */
    flip_button = gtk_button_new_with_label("Flip Board"); /* Flip board button */
    
    // Texts and Text boxes.
    depth_text = gtk_label_new("Depth Searched - 0"); /* Create a new label */
    eval_text = gtk_label_new("Evaluation - 0"); /* Create a new label */
    move_text = gtk_label_new("Last Move - N/A"); /* Create a new label */
    side_text = gtk_label_new(state->side ? "Side To Move - White" : "Side to Move - Black"); /* Create a new label */
    think_text = gtk_label_new(""); /* Create a new label */
    
    // Set these in the state */
    state->depth_text = depth_text; 
    state->eval_text = eval_text; 
    state->move_text = move_text; 
    state->side_text = side_text; 
    state->think_text = think_text; 
    // Text Boxes
    
    depth_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* To style and size the text */
    eval_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* To style and size the text */
    move_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* To style and size the text */
    side_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* To style and size the text */
    think_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); /* To style and size the text */
    
    // Format Widgets
    gtk_window_set_title(GTK_WINDOW (*game_window), "The Cactus - a chess AI that is supposed to defeat humans in chess."); /* Set the window title */
    gtk_window_set_default_size(GTK_WINDOW (*game_window), 1200, 600); /* Set the window size */
    gtk_label_set_line_wrap(GTK_LABEL(move_text), 1); /* Set wrap if text is too big */
    gtk_widget_set_size_request(move_text, 200, -1); /* Set position to wrap */ 
    gtk_widget_set_size_request(side_container, 600, -1); /* Set width of side box */
    gtk_box_set_homogeneous(GTK_BOX(container), 0); /* Non-homogenous */
    gtk_label_set_use_markup(GTK_LABEL(think_text), TRUE); /* Set use markup to true (to help make font bigger) */
    // Connect Signals
    gtk_widget_add_events(*board_canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK); /* Add the mouse move event to the board */
    g_signal_connect(*board_canvas, "draw", G_CALLBACK(draw_board), state); /* Draw the board */ 
    g_signal_connect(flip_button, "clicked", G_CALLBACK(flip_board_callback), state); /* Draw the board */
    g_signal_connect(*board_canvas, "motion-notify-event", G_CALLBACK(mouse_move_callback), state); /* Mouse position track */ 
    g_signal_connect(*board_canvas, "button-press-event", G_CALLBACK(mouse_down_callback), state); /* Click button down */ 
    g_signal_connect(*board_canvas, "button-release-event", G_CALLBACK(mouse_up_callback), state); /* Click button down */ 
    
    // Pack Widgets

    // Pack Text boxes.
    gtk_box_pack_start(GTK_BOX(side_box), side_text, 1, 1, 20); /* Add the text to text box */
    gtk_box_pack_start(GTK_BOX(think_box), think_text, 1, 1, 20); /* Add the text to text box */
    gtk_box_pack_start(GTK_BOX(depth_box), depth_text, 1, 1, 20); /* Add the text to text box */
    gtk_box_pack_start(GTK_BOX(eval_box), eval_text, 1, 1, 20); /* Add the text to text box */
    gtk_box_pack_start(GTK_BOX(move_box), move_text, 1, 1, 20); /* Add the text to text box */
    
    // Pack boxes into container
    gtk_box_pack_start(GTK_BOX(side_container), side_box, 0, 0, 20); /* Add text box */
    gtk_box_pack_start(GTK_BOX(side_container), depth_box, 0, 0, 20); /* Add text box */
    gtk_box_pack_start(GTK_BOX(side_container), eval_box, 0, 0, 20); /* Add text box */
    gtk_box_pack_start(GTK_BOX(side_container), move_box, 0, 0, 20); /* Add text box */
    gtk_box_pack_start(GTK_BOX(side_container), think_box, 0, 0, 20); /* Add text box */

    gtk_container_add(GTK_CONTAINER(*game_window), container); /* Add the row */
    gtk_box_pack_start(GTK_BOX(container), *board_canvas, 1, 1, 0); /* Add the board canvas */
    gtk_box_pack_start(GTK_BOX(container), side_container, 0, 0, 0); /* Add the side container */
    gtk_box_pack_start(GTK_BOX(side_container), button_box, 0, 0, 30); /* Add the flip button */
    gtk_box_pack_start(GTK_BOX(button_box), flip_button, 1, 1, 20); /* Add the flip button */
    gtk_widget_show_all(*game_window); /* Show the window */
}

void update_metadata_log(GameState *state) { /* Log Game Metadata */
    FILE *log = fopen(state->log_filename, "w"); /* Open Logfile */
    char date_string[20]; /* Date */
    time_t current_time = time(NULL); /* Get time to calculate date*/
    strftime(date_string, 20, "%d-%m-%Y", localtime(&current_time));
    // Write metadata
    fprintf(log, "Date, %s,,,,,\nCactus Plays, %s,,,,,\nHuman Plays, %s,,,,,\nNo., Side, Key, Move, Move Desc., Depth, Evaluation\n",
            date_string, state->human_side ? "Black" : "White", state->human_side ? "White" : "Black"
        );
    fclose(log); /* Close Log File */

}

void start_game(GtkApplication **app, Bitboard *board, int human_side, int think_time, char *log_filename) {
    /* Starts the game with the GUI */
    // Declare everything
    GameState *state = (GameState*)malloc(sizeof(GameState));
    // GUI Components
    GtkWidget *game_window; /* The gui window of the game */
    GtkWidget *board_canvas; /* The canvas to draw the board on */
    // Setup Game State.
    state->board = board; /* Set the game state board */
    state->think_time = think_time; /* Engine Think Time */
    state->human_side = human_side; /* Set the human side */
    state->perspective = human_side; /* For now, let the perspective be the same as the human side */
    state->game_over = 0; /* Game not yet over */
    state->log_filename = log_filename; /* Set Logging Filename */
    if (state->log_filename) update_metadata_log(state); /* Write beginning metadata */
    update_game_state(state, 0, 0, 0, 0); /* Update everything else in the game state */
    
    create_game_window(app, &game_window, &board_canvas, state); /* Create the game window */
    if (human_side != board->side) launch_engine_thread(state); /* If engine to play, let it start thinking */
    
}

struct launch_action_data {
    Bitboard *board;
    int human_side;
    int search_time;
    char *log_filename;
};

void launch_action(GtkApplication *app, struct launch_action_data *data) {
    /* What to do when the game is launched */
    start_game(&app, data->board, data->human_side, data->search_time, data->log_filename); /* Start game */
}

void launch_options_action(GtkApplication *app, struct launch_action_data *data) {
    /* This will launch a GUI, but with multiple options 
     * These options will include:
     *  -> Side to play
     *  -> Search Time
     *  -> Custom Starting FEN
     *  -> Log File Path
    */

    Bitboard *board = data->board;

    GtkWidget *popup_dialog;
    GtkWidget *content_grid;
    GtkWidget *popup_content; /* The content of the popup */
    
    int response_id;

    popup_dialog = gtk_dialog_new_with_buttons( /* Create an interactive dialog with buttons */
            "The Cactus - a chess AI that is supposed to defeat humans in chess.", /* Dialog title */
            NULL, /* Toplevel window */
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, /* Flags */
            /* Here come the buttons */
            "_Play As White", /* Button title */
            1, /* Response id */
            "_Play As Black", 0,
            "_Cancel", /* Ditto */
            2,
            NULL /* List ends with null */
            );
    
    // Create inside of popup
    popup_content = gtk_dialog_get_content_area(GTK_DIALOG (popup_dialog)); /* Where to place the content of the box */
    content_grid = gtk_grid_new(); /* Grid that includes all the entrys and labels */
    
    // Declare Widgets
    GtkWidget *search_time;
    GtkWidget *log_file;
    GtkWidget *custom_fen;
    GtkAdjustment *think_time_adjustment;

    // Declare Labels
    GtkWidget *time_label;
    GtkWidget *log_label;
    GtkWidget *fen_label;    
    
    // Create Labels
    time_label = gtk_label_new("Think Time"); /* Create the label */
    log_label = gtk_label_new("Log File"); /* Create the label */
    fen_label = gtk_label_new("Custom Position (FEN)"); /* Create the label */

    // Create Inputs
    think_time_adjustment = gtk_adjustment_new(10, 0, 86400, 1, 1, 1); /* An adjusment, simply the value */
    search_time = gtk_spin_button_new(think_time_adjustment, 1, 0); /* Think time actual widget */
    log_file = gtk_file_chooser_button_new("Select File", GTK_FILE_CHOOSER_ACTION_OPEN); /* Select File Button */
    custom_fen = gtk_entry_new(); /* Create a new entry for a custom FEN string */
    

    // Format Widgets 
    gtk_grid_set_row_spacing(GTK_GRID (content_grid), 4); /* Set space between rows in the grid */
    gtk_grid_set_column_spacing(GTK_GRID (content_grid), 4); /* Set space between columns in the grid */
    g_object_set(G_OBJECT(content_grid), "margin", 10, (gchar*)0 /* Null-terminated arguments */); /* Add padding of 10px */
    
    // Pack Widgets
    gtk_grid_attach(GTK_GRID (content_grid), time_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID (content_grid), search_time, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID (content_grid), log_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID (content_grid), log_file, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID (content_grid), fen_label, 0, 2, 1, 2);
    gtk_grid_attach(GTK_GRID (content_grid), custom_fen, 1, 2, 1, 2);
    
    /* pack the grid into the popup */
    gtk_container_add(GTK_CONTAINER (popup_content), content_grid); /* Add the grid into the popup */
    gtk_widget_show_all(popup_content); /* Show the widgets inside the popup */

    // Run the dialog box
    response_id = gtk_dialog_run(GTK_DIALOG (popup_dialog)); /* Open the dialog */
    
    if (response_id != 1 && response_id != 0) return; /* If they click cancel, do nothing */

    /* All parameters that need to be set for working */
    int human_side = response_id; /* The side that the human will play */
    int think_time = 10; /* Time given for engine to think */
    int use_custom_fen = 0; /* Whether to start from custom position or not */
    gchar *custom_fen_string; /* Custom Position */
    gchar *log_file_path; /* Ok I think that's enough for a log file path */
    int to_log = 0; /* Log this game or not */

    // Get the values 
    think_time = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(search_time)); /* Get the search time from the spin button */
    custom_fen_string = (gchar*)gtk_entry_get_text(GTK_ENTRY(custom_fen)); /* Get the fen string text */
    if (strlen(custom_fen_string) > 0) use_custom_fen = 1; /* If there is a string, use it */
    log_file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(log_file)); /* Get the file path */
    if (log_file_path) to_log = 1; /* If the log file path is set, set to log */
    
    if (use_custom_fen) parse_fen(board, (char*)custom_fen_string); /* If the custom fen is set, use it */
    
    gtk_widget_destroy(popup_dialog); /* Close Popup */

    start_game(&app, board, human_side, think_time, to_log ? log_file_path : 0); /* Start the game with specified details */
}

int launch_options(Bitboard *board, int argc, char **argv) {
    /* This will launch the chess engine, but with options */
    GtkApplication *app; /* This is the app object that will be used for creating windows */
    app = gtk_application_new("io.github.aaranyak.the_cactus", G_APPLICATION_FLAGS_NONE); /* Create new GTK Application with id "io.github.aaranyak.the_cactus" */
    struct launch_action_data *signal_data = (struct launch_action_data*)malloc(sizeof(struct launch_action_data)); /* Allocate data memory */
    signal_data->board = board; /* Only send the board */
    g_signal_connect_data(app, "activate", G_CALLBACK (launch_options_action), signal_data, (GClosureNotify)free, 0); /* Set app start callback to the start function */
    int status = g_application_run(G_APPLICATION (app), 0, 0); /* Run app */
    g_object_unref(app); /* Free memory when app is closed */
    return status;
}

int launch_gui(Bitboard *board, int argc, char **argv, int human_side, int search_time, char *log_filename) {
    /* Launches the GUI for the game */
    GtkApplication *app; /* This is the app object that will be used for creating windows */
    app = gtk_application_new("io.github.aaranyak.the_cactus", G_APPLICATION_FLAGS_NONE); /* Create new GTK Application with id "io.github.aaranyak.the_cactus" */
    struct launch_action_data *signal_data = (struct launch_action_data*)malloc(sizeof(struct launch_action_data)); /* Allocate data memory */
    signal_data->board = board; signal_data->search_time = search_time; signal_data->human_side = human_side; signal_data->log_filename = log_filename; /* Set data values */
    g_signal_connect_data(app, "activate", G_CALLBACK (launch_action), signal_data, (GClosureNotify)free, 0); /* Set app start callback to the start function */
    int status = g_application_run(G_APPLICATION (app), 0, 0); /* Run app */
    g_object_unref(app); /* Free memory when app is closed */
    return status;
}
