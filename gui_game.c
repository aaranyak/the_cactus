/* gui_game.c 
 * Graphical user interface built using windows api
*/
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <time.h>
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

LRESULT CALLBACK window_process(HWND window_handle, UINT messages, WPARAM w_params, LPARAM l_params); /* Forward Decleration to window function */
void draw_board(GameState *state, HWND window_handle, PAINTSTRUCT *canvas, HDC *brush); /* Forward decleration for drawing board function */

struct thread_params *data;

HBRUSH white_square;
HBRUSH black_square;

HBRUSH from_square;
HBRUSH to_square;
void play_move_on_board(GameState *state, move_t move);

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

void update_game_state(GameState *state, move_t last_move) {
    /* Update the current game state */
    state->side = state->board->side; /* Update the side to move */
    // Set squares
    for (int square = 0; square < 64; square++) { /* Loop through all squares */
        state->mailbox[square] = -1; /* Nothing in it (yet) */
        for (int piece = 0; piece < 12; piece++) { /* Loop through all the pieces */
            U64 squareBB = 1ULL << square;
            if (state->board->pieces[piece] & squareBB) state->mailbox[square] = piece;
        }
    }

    // Clear Selection
    state->is_selected = 0;
    state->is_hovering = 0;
    state->selected_position = 0;
    state->hover_pos_x = 0;
    state->hover_pos_y = 0;

    state->last_move = last_move;

    state->legal_moves = generate_legal_moves(state->board); /* Pre-generate legal moves */
}

void redraw_window(HWND window_handle, int rect) {
        RECT window_rect;
        GetWindowRect(window_handle, &window_rect); /* Window Rect */
        InvalidateRect(window_handle, rect ? &window_rect : 0, 0); /* Redraw Window */
}
void init_game_state(GameState *state, Bitboard *board, int human_side, int search_time, HINSTANCE app_id) {
    /* Initializes the state of the game */
    // Set the basic variables of the state
    state->board = board;
    state->human_side = human_side;
    state->search_time = search_time;
    state->perspective = human_side;
    state->app_id = app_id;
    state->game_over = 0;
    update_game_state(state, 0); /* Update the game state */
}
struct thread_params {
    GameState *state;
    HWND window_handle;
};

void *engine_think(void *raw_data) {
    /* Engine think function */
    struct thread_params *data = (struct thread_params*)raw_data;
    GameState *state = data->state;
    HWND window_handle = data->window_handle;
    id_result_t result; /* Search result */
    result = iterative_deepening(state->board, state->search_time);
    printf("Searched to a depth of %d, Evaluation %d\n", result.depth, result.evaluation);
    play_move_on_board(state, result.move); /* Play the move on the board, and update values */
    redraw_window(window_handle, 0); /* Redraw Window */
    free(data); /* Free the thread data */
}

void launch_engine_thread(GameState *state, HWND window_handle) {
    /* Launches the thread for the engine to think */
    if (!state->game_over) {
        data = (struct thread_params*)malloc(sizeof(struct thread_params)); /* Data to pass onto the thread */
        data->state = state;
        data->window_handle = window_handle;
        HANDLE thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)engine_think, data, 0, 0); /* Launch the thread */
    }
}

int launch_gui(HINSTANCE app_id, int show_cl, Bitboard *board, int human_side, int search_time) {
    /* Launches the GUI using the Windows API */
    
    // Set up the game state
    GameState *state = (GameState*)malloc(sizeof(GameState)); /* Create a game state object */
    init_game_state(state, board, human_side, search_time, app_id);
    // Set up the window class
    const char CLASS_NAME[] = "Cactus GUI Window"; /* Name of the window class (Windows programming is HARD...) */
    WNDCLASS window_class = {}; /* Just put nothing in it for now */
    window_class.lpfnWndProc = window_process; /* This function probably contains most of the processes to do with the window */
    window_class.hInstance = app_id; /* The application instance */
    window_class.lpszClassName = CLASS_NAME; /* Name of the class */
    RegisterClass(&window_class); /* Why can't you just keep the syntax style constant */
    white_square = CreateSolidBrush(RGB(111, 143, 114));
    black_square = CreateSolidBrush(RGB(173, 189, 143));
    from_square = CreateSolidBrush(RGB(107, 202, 93));
    to_square = CreateSolidBrush(RGB(55, 110, 47));


    // Create a window
    HWND window_handle = CreateWindowEx(
        0, /* Optionall window styles */
        CLASS_NAME, /* Class Name */
        "The Cactus - a chess AI that is supposed to defeat humans in chess", /* Window title */
        WS_OVERLAPPEDWINDOW, /* Window Style */
        CW_USEDEFAULT, CW_USEDEFAULT, 577, 600, /* Size and position */
        NULL, /* Parent Window */
        NULL, /* Menu */
        app_id, /* App Handle */
        state /* App Data */
    ); /* Why can't you make the names prononunceable... */
    
    if (!window_handle) return 1; /* If it didn't work */

    // Show thw window
    ShowWindow(window_handle, show_cl);
    if (human_side == 0) launch_engine_thread(state, window_handle);
    // Window Mainloop (also known as message loop)
    MSG message = {}; /* The Message */
    int last_update = 0; /* Last frame updated */
    int current_time; /* Current Time */
    int time_difference = 10; /* Approx 60fps */
    RECT window_rect;
    while (message.message != WM_QUIT) { /* As long as the quit button is not clicked */
        // Handle Messages 
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message); /* I don't exactly know what this does. */
            DispatchMessage(&message); /* https://learn.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-dispatchmessage will probably explain more */
        }
        current_time = clock() / (CLOCKS_PER_SEC / 1000);
        if (current_time > last_update + time_difference) {
            last_update = current_time;
        }
    }
    free(state); /* Free the game state */
    return 0;
}

void play_move_on_board(GameState *state, move_t move) {
    /* Play a move on the board, and update status */
    if (!state->game_over) {
        U64 a, b, c; int d; /* Saved data stuff for move */
        make_move(state->board, move, &a, &b, &c, &d); /* Make the move on the board */
        update_game_state(state, move); /* Update the game state with the last move */
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
                state->game_over = 1; /* Game Over (Stalemate) */
            }
        }
    }
}

void play_human_move(GameState *state, int index, HWND window_handle) {
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
                int response = -1;
                char piece_messages[4][3000] = {"Would you like to promote to a ROOK?", "Would you like to promote to a KNIGHT?", "Would you like to promote to a BISHOP?", "Would you like to promote to a QUEEN?"};
                for (int piece = 0; piece < 4; piece++) {
                    int message_id = MessageBox(
                                NULL,
                                piece_messages[piece],
                                "Promote to:",
                                MB_ICONQUESTION | MB_YESNO
                            );
                    if (message_id == IDYES) {
                        response = piece;
                        break;
                    }
                }
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
        play_move_on_board(state, is_move);
        launch_engine_thread(state, window_handle); /* Let the AI start thinking */
    }
}

void set_resize(GameState *state, HWND window_handle, int width, int height) {
    /* Handle resizing */
    state->height = height; /* Square Height */
    state->width = width; /* Square Width */
    redraw_window(window_handle, 0);
}

void handle_mouse_move(GameState *state, HWND window_handle, UINT message, WPARAM w_params, LPARAM l_params) {
    /* Handler for mouse movement */
    state->hover_pos_x = GET_X_LPARAM(l_params); /* Set the hover x position to the mouse position */
    state->hover_pos_y = GET_Y_LPARAM(l_params); /* Set the hover y position to the mouse position */
    redraw_window(window_handle, 0); /* Test */
}

void handle_mouse_down(GameState *state, HWND window_handle, UINT message, WPARAM w_params, LPARAM l_params) {
    /* Handle mouse down (Selection and hovering) */
    if (!state->game_over) {
        /* If game not over */
        if (state->side == state->human_side) {
            /* If human to move */
            // Calculate piece
            int board_size = min(state->width, state->height); /* Board size is min of width and height */
            float square_size = board_size / 8.0f; /* Size of each square on the board */
            int file = state->hover_pos_x / square_size; /* Get the file that the mouse is on */
            int rank = state->hover_pos_y / square_size; /* Get the rank that the mouse is on */
            int index = state->perspective ? file + ((7 - rank) * 8) : (7 - file) + (rank * 8); /* Get the square index */
            int piece = state->mailbox[index];

            // Select piece
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
            redraw_window(window_handle, 0); /* Update drawing area */      
        }
    }
}

void handle_mouse_up(GameState *state, HWND window_handle, UINT message, WPARAM w_params, LPARAM l_params) {
    /* Handle mouse down (Selection and hovering) */
    if (!state->game_over) {
        /* If game not yet over */
        if (state->side == state->human_side) { /* Human to move */
            if (state->is_selected) { /* Piece is selected */
                // For now just stop hovering, we can deal with all that stuff later
                int board_size = min(state->width, state->height); /* Board size is min of width and height */
                float square_size = board_size / 8.0f; /* Size of each square on the board */
                int file = state->hover_pos_x / square_size; /* Get the file that the mouse is on */
                int rank = state->hover_pos_y / square_size; /* Get the rank that the mouse is on */
                int index = state->perspective ? file + ((7 - rank) * 8) : (7 - file) + (rank * 8); /* Get the square index */
                int piece = state->mailbox[index];
                state->is_hovering = 0; /* Stop hovering */
                
                // Check if this is a legal move, if so make it on the board */
                play_human_move(state, index, window_handle); /* Play the move */
                redraw_window(window_handle, 0); /* Isn't it obvious */      
            }
        }
    }
}
LRESULT CALLBACK window_process(HWND window_handle, UINT message, WPARAM w_params, LPARAM l_params) {
    /* Window Process */
    GameState *state = (GameState*)GetWindowLongPtr(window_handle, GWLP_USERDATA); /* OK these names are really getting on my nerves */

    switch (message) { /* Handle Message */
        case WM_PAINT: { /* If this is a window paint message */
            // Draw Window
            PAINTSTRUCT canvas; /* The Canvas */
            HDC brush = BeginPaint(window_handle, &canvas); /* Begin painting with the brush */
            HDC memory_brush = CreateCompatibleDC(brush); /* HDC in memory */
            HBITMAP memory_bitmap = CreateCompatibleBitmap(brush, state->width, state->height); /* Create Buffer */
            HANDLE hold = SelectObject(memory_brush, memory_bitmap); /* Connect it */
            FillRect(memory_brush, &canvas.rcPaint, (HBRUSH)(COLOR_WINDOW + 1)); /* Paint a rectangle */
            /* <Do Stuff Area> */
            draw_board(state, window_handle, &canvas, &memory_brush); /* Draw the board */
            BitBlt(brush, 0, 0, state->width, state->height, memory_brush, 0, 0, SRCCOPY); /* Paste Directly */
            /* </Do Stuff Area> */
            SelectObject(memory_brush, hold); /* Give it back */
            DeleteObject(memory_bitmap); /* Return the bitmap */
            DeleteDC(memory_brush); /* Throw it away */
            EndPaint(window_handle, &canvas); /* Stop Painting */
            break;
        }
        case WM_CREATE: { /* On creation, pass data onto window */
            CREATESTRUCT *create = (CREATESTRUCT*)l_params; /* Contains data */
            state = (GameState*)create->lpCreateParams; /* Get the user data */
            SetWindowLongPtr(window_handle, GWLP_USERDATA, (LONG_PTR)state); /* Set the state */
            break;
        }
        case WM_SIZE: {
            set_resize(state, window_handle, LOWORD(l_params), HIWORD(l_params)); /* Set the size of the squares */
            break;
        }
        case WM_MOUSEMOVE: { /* Handle Mouse Movement */
            handle_mouse_move(state, window_handle, message, w_params, l_params); /* Handle mouse movement */
            break;
        }
        case WM_LBUTTONDOWN: {
            handle_mouse_down(state, window_handle, message, w_params, l_params); /* Handle mouse down */
            break;
        }
        case WM_LBUTTONUP: {
            handle_mouse_up(state, window_handle, message, w_params, l_params); /* Handle mouse down */
            break;
        }
        case WM_CHAR: {
            if (w_params == 'f') { /* Flip Board */
                state->perspective = !state->perspective;
                redraw_window(window_handle, 0); 
            }
            break;
        }
        case WM_DESTROY: { /* Close Window */
            PostQuitMessage(0); /* Exit this */
            return 0; /* Exit with 0 */
        }
    }
    return DefWindowProc(window_handle, message, w_params, l_params); /* Regular Exit */
}

void draw_board(GameState *state, HWND window_handle, PAINTSTRUCT *canvas, HDC *brush) {
    /* Draw the board */
    char piece_icons[12][20] = {"♜", "♞", "♝", "♛", "♚", "♟", "♜", "♞", "♝", "♛", "♚", "♟"}; /* Icons */
    float size = min(state->width, state->height) / 8.0f; /* For easier wiriting */
    HFONT piece_font = CreateFont(size / 1.16, 0, 0, 0, FW_DONTCARE /* This is probably the single best named constant in the entirity of this API */, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH | FF_ROMAN, TEXT(""));
    HFONT text_font = CreateFont(50, 0, 0, 0, FW_DONTCARE /* This is probably the single best named constant in the entirity of this API */, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH | FF_ROMAN, TEXT("Times New Roman"));
    HFONT old_font; /* I hope this works */
    RECT square_rect; /* Square Rect */
    // Draw the board squares
    int rank; /* For looping over ranks */
    int file; /* For looping over files */
    int colour = 0; /* Colour */
    int index; /* Index of current square */
    int piece; /* Current Piece */
    for (rank = 0; rank < 8; rank++) { /* Loop over all ranks */
        for (file = 0; file < 8; file++) { /* Loop over all files */
            colour = (file + rank) % 2; /* Square Colour */
            index = state->perspective ? file + ((7 - rank) * 8) : (7 - file) + (rank * 8); /* Get the square index */
            // Set square rect
            square_rect.left = file * size; /* Left */
            square_rect.top = rank * size; /* Top */
            square_rect.right = file * size + size; /* Right */
            square_rect.bottom = rank * size + size; /* Bottom */
            if (index == ((state->last_move & MM_FROM) >> MS_FROM)) {
                FillRect(*brush, &square_rect, from_square);
            }
            else if (index == ((state->last_move & MM_TO) >> MS_TO)) {
                FillRect(*brush, &square_rect, to_square);                
            }
            else FillRect(*brush, &square_rect,colour ?  white_square : black_square);
            
            // Draw the pieces
            piece = state->mailbox[index]; /* Get the current piece */
            if (piece != -1 && !(state->is_hovering && state->is_selected && index == state->selected_position && piece == state->selected_piece)) { /* If there is a piece on this square */
                // Draw the piece as text
                old_font = SelectObject(*brush, piece_font); /* Set the font */
                SetTextColor(*brush, (piece < 6) ? RGB(255, 255, 255) : RGB(0, 0, 0)); /* Set the piece icon colour */
                SetBkMode(*brush, TRANSPARENT); /* Make background transparent */
                DrawText(*brush, piece_icons[piece], -1, &square_rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                SelectObject(*brush, old_font); /* To prevent leaks */
            }
        }
    }

    if (state->is_selected && state->side == state->human_side) { /* If there is a selected piece */
        // Display all the possible squares it can move to */
        int move;
        int to;
        SelectObject(*brush, GetStockObject(DC_BRUSH)); /* Connect to brush */
        SetDCBrushColor(*brush, RGB(9, 87, 3)); /* Set brush colour */
        SelectObject(*brush, GetStockObject(DC_PEN)); /* Connect to brush */
        SetDCPenColor(*brush, RGB(9, 87, 3)); /* Set brush colour */
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
                    Ellipse(*brush, file * size + (size / 3), rank * size + (size / 3), file * size + size - (size / 3), rank * size + size - (size / 3)); /* Actually draw the circle */
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
                Ellipse(*brush, file * size + (size / 3), rank * size + (size / 3), file * size + size - (size / 3), rank * size + size - (size / 3)); /* Actually draw the circle */

            }
        }
    }

    // Draw hovering piece
    if (state->is_selected && state->is_hovering) { /* If there is a hvovering piece */
        // Draw the piece as text
        RECT hovering_rect = (RECT){state->hover_pos_x - (size / 2), state->hover_pos_y - (size / 2), state->hover_pos_x + size - (size / 2), state->hover_pos_y + size - (size / 2)};
        old_font = SelectObject(*brush, piece_font); /* Set the font */
        SetTextColor(*brush, (state->selected_piece < 6) ? RGB(255, 255, 255) : RGB(0, 0, 0)); /* Set the piece icon colour */
        SetBkMode(*brush, TRANSPARENT); /* Make background transparent */
        DrawText(*brush, piece_icons[state->selected_piece], -1, &hovering_rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
        SelectObject(*brush, old_font); /* To prevent leaks */
    }

    if (state->game_over) {
        if (state->draw) {
            old_font = SelectObject(*brush, text_font); /* Set the font */
            SetTextColor(*brush, RGB(0, 0, 0)); /* Set the piece icon colour */
            SetBkMode(*brush, TRANSPARENT); /* Make background transparent */
            DrawText(*brush, "Ends in a Draw.", -1, &(RECT){0,0,min(state->width, state->height),min(state->width, state->height)}, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            SelectObject(*brush, old_font); /* To prevent leaks */
        }
        else if (state->white_wins) {
            old_font = SelectObject(*brush, text_font); /* Set the font */
            SetTextColor(*brush, RGB(0, 0, 0)); /* Set the piece icon colour */
            SetBkMode(*brush, TRANSPARENT); /* Make background transparent */
            DrawText(*brush, "White Checkmates Black!", -1, &(RECT){0,0,min(state->width, state->height),min(state->width, state->height)}, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            SelectObject(*brush, old_font); /* To prevent leaks */
        }
        else {
            old_font = SelectObject(*brush, text_font); /* Set the font */
            SetTextColor(*brush, RGB(0, 0, 0)); /* Set the piece icon colour */
            SetBkMode(*brush, TRANSPARENT); /* Make background transparent */
            DrawText(*brush, "Black Checkmates White!", -1, &(RECT){0,0,min(state->width, state->height),min(state->width, state->height)}, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            SelectObject(*brush, old_font); /* To prevent leaks */
        }
    }

    DeleteObject(piece_font); /* Delete this font for good measure */
    DeleteObject(text_font); /* Delete this font for good measure */
}