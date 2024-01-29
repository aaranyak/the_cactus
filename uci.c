/* uci.c
 * This is the code for the UCI protocol.
 * This protocol works with most chess GUIs.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
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
#define INF INT_MAX

int get_word(char input[], char output[], int start) {
    /* This will basically get a single word from the start index */
    
    int index;
    int char_index = 0;
    for (index = start; 1; index++) { /* Loop through the letters */
        if (input[index] == 0 || input[index] == ' ') break; /* Until reached a space or end of string */
        output[char_index] = input[index]; /* Add it */
        char_index++;
    }

    output[char_index] = 0;

    return index + 1;
}

void generate_move_string(Bitboard *board, move_t move, char out[]) {
    /* Generate the move string */
    if (move & MM_CAS) { /* If this is a castling move */
        if (board->side) { /* For white */
            if (move & MM_CSD) sprintf(out, "e1c1"); /* White queenside castle */
            else sprintf(out, "e1g1"); /* White kingside castling */
        } else { /* For Black */
            if (move & MM_CSD) sprintf(out, "e8c8"); /* Black queenside */
            else sprintf(out, "e8g8"); /* Good.. Handled castling */
        }
        return;
    }
    
    char start_file;
    char start_rank;
    char end_file;
    char end_rank;

    int start = (move & MM_FROM) >> MS_FROM;
    int end = (move & MM_TO) >> MS_TO; /* To square */
    

    char filenames[8] = "abcdefgh";
    char ranknames[8] = "12345678";
    start_file = filenames[(start % 8)]; /* The start file */
    start_rank = ranknames[(start / 8)]; /* start rank */
    end_file = filenames[(end % 8)]; /* End file */
    end_rank = ranknames[(end / 8)]; /* End square rank */
    /* This is getting pretty boring.. ..isn't it? Don't worry, we'll get to the interesting part later */
    
    char pawnpromotions[4] = {'r', 'n', 'b', 'q'}; /* Pawn Promotion Pieces */
    
    if (move & MM_PRO) { /* If this is a promotion move */
        sprintf(out, "%c%c%c%c%c", start_file, start_rank, end_file, end_rank, pawnpromotions[(move & MM_PPP) >> MS_PPP]); /* Promotion Move String */
    } else {
        sprintf(out, "%c%c%c%c", start_file, start_rank, end_file, end_rank); /* Regular move string */
    }
}

move_t parse_move(Bitboard *board, char move_string[]) {
    /* Basically get a good move */

    char current_move_string[64] = {0};

    move_list_t moves = generate_legal_moves(board);

    for (int index = 0; index < moves.count; index++) {
        generate_move_string(board, moves.moves[index], current_move_string);
        if (!strcmp(current_move_string, move_string)) return moves.moves[index];
    }
    return 0;
}

void parse_position(Bitboard *board, char input_line[], char output_line[], int *engine_running, int *thinking, int *ready_command_sent, int next_index) {
    /* Parses the position command */
    
    // Declare...
    char position_type[64] = {0}; /* This can either be "startpos" or "fen" */
    char fen_string[256] = {0}; /* The FEN string */
    int fen_index;
    next_index = get_word(input_line, position_type, next_index); /* Get the fen string */
    if (!strcmp(position_type, "fen")) { /* Well, you've just got to deal with it */
        /* Parse the FEN string */
        if (input_line[next_index - 1] != 0) {
            for (fen_index = 0; 1; fen_index++) { /* Until the fen string is over */
                if (input_line[next_index] == 0 || input_line[next_index] == 'm') break; /* Until fen is over */
                fen_string[fen_index] = input_line[next_index]; /* Set part of the fen string */
                next_index++; /* Next char */
            }
            parse_fen(board, fen_string); /* Parse the FEN string into the board */
        }
    } else if (!strcmp(position_type, "startpos")) {
        /* Classical chess starting */
        parse_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0"); /* This is it, the startposition */
    } else {
        /* Do nothing */
        return;
    }

    // Parse moves.
    if (input_line[next_index - 1] != 0) { /* String not yet over */
        char next_word[256] = {0}; /* To check what the next word is */
        next_index = get_word(input_line, next_word, next_index); /* Get the next word (supposed to be "moves") */
        move_t current_move = 0;
        U64 a, b, c; int d; /* These are saved stuff */
        if (!strcmp(next_word, "moves")) { /* If the next word is "moves" */
            /* Loop through each move */
            while (1) { /* Until the last move has been reached (I seem to be using quite a few infinite loops around here */
                if (!input_line[next_index]) break; /* Reached end of the line */
                next_index = get_word(input_line, next_word, next_index); /* Get the next move */
                current_move = parse_move(board, next_word); /* Parse the move */
                make_move(board, current_move, &a, &b, &c, &d); /* Make the move on the board */
            }
        }
    }
}

int get_search_time(int time_remaining, int increment) {
    /* Simple time management function */
    
    return (time_remaining / 40) + (increment / 2); /* Basic time management */
}

struct think_data {
    Bitboard *board;
    int think_time;
    int *thinking;
    int *ready_command_sent;
};

void *think_thread_function(void *think_data_uncasted) {
    /* Function to make AI start thinking */
    // Unpack Data
    struct think_data *data = (struct think_data*)think_data_uncasted; /* Correct Type */
    Bitboard *board = data->board;
    int think_time = data->think_time;
    int *thinking = data->thinking;
    int *ready_command_sent = data->ready_command_sent;
    
    free(data); /* Because we don't need this anymore */

    // Declare
    id_result_t result;
    result = iterative_deepening(board, think_time); /* start thinking */ 

    *thinking = 0; /* Stop thinking */

    char move_string[64] = {0}; /* Will contain the move name */
    generate_move_string(board, result.move, move_string); /* Get the string */
    setbuf(stdout, NULL); /* Somehow it makes it magically work */
    printf("info depth %d time %d score cp %d\n", result.depth, think_time, result.evaluation); /* Debugging data */
    printf("info string Get Poked by the Kaktus Poke!\n"); /* Seriously */
    printf("bestmove %s\n", move_string); /* Send best move as well */
    
    if (*ready_command_sent) {
        /* Said isready before */
        printf("readyok\n");
        *ready_command_sent = 0;
    }
}

void run_thinking_thread(Bitboard *board, int think_time, int *thinking, int *ready_command_sent) {
    /* Pack data and send to thinking thread */
    struct think_data *data = (struct think_data*)malloc(sizeof(struct think_data)); /* Allocate data memory */
    // Pack data into struct
    data->board = board;
    data->think_time = think_time;
    data->thinking = thinking;
    data->ready_command_sent = ready_command_sent;
    // Start a thread
    pthread_t think_thread;
    int retval = pthread_create(&think_thread, NULL, think_thread_function, data); /* Start the thread */
} 

void parse_go(Bitboard *board, char input_line[], char output_line[], int *engine_running, int *thinking, int *ready_command_sent, int next_index) {
    /* Parse the "go" command */
    
    int think_time = 10; /* This is the think time given */

    int w_remaining = -1; /* White Remaining Think Time */
    int b_remaining = -1; /* Ditto for black */
    
    int w_increment = 0;
    int b_increment = 0;

    int fixed_time = -1; /* Fixed time search */
    
    char current_word[64] = {0}; /* The current word */

    while (1) { /* Until the sentence is over */
        if (input_line[next_index - 1] == 0) break; /* Done... */
        next_index = get_word(input_line, current_word, next_index); /* Get the next word */
        
        if (!strcmp(current_word, "wtime")) {
            next_index = get_word(input_line, current_word, next_index); /* Get the white time remaining */
            sscanf(current_word, "%d", &w_remaining); /* Get the time remaining */
        } else if (!strcmp(current_word, "btime")) {
            next_index = get_word(input_line, current_word, next_index); /* Get the black time remaining */
            sscanf(current_word, "%d", &b_remaining); /* Get the time remaining */
        } else if (!strcmp(current_word, "winc")) {
            next_index = get_word(input_line, current_word, next_index); /* Get the white increment */
            sscanf(current_word, "%d", &w_increment); /* Get the increment */
        } else if (!strcmp(current_word, "binc")) {
            next_index = get_word(input_line, current_word, next_index); /* Get the black increment */
            sscanf(current_word, "%d", &b_increment); /* Get the increment */
        } else if (!strcmp(current_word, "movetime")) {
            next_index = get_word(input_line, current_word, next_index); /* Get the fixed move time*/
            sscanf(current_word, "%d", &fixed_time); /* Get the time */
        }
    }
    
    if (fixed_time != -1) {
        /* Maximum time given */
        think_time = (fixed_time / 1000) - 1; /* Calculate fixed think time */
    }
    else if ((board->side ? w_remaining : b_remaining) != -1) { /* Regular time given */
        think_time = get_search_time(board->side ? w_remaining : b_remaining, board->side ? w_increment : b_increment) / 1000; /* Calculate think time */
    }

    *thinking = 1; /* Currently thinking */
    sprintf(output_line, "info string Starting to think...\n");
    run_thinking_thread(board, think_time, thinking, ready_command_sent); /* Run thread */
    
} 

void parse_uci_command(Bitboard *board, char input_line[], char output_line[], int *engine_running, int *thinking, int *ready_command_sent) {
    /* Parses the UCI command, based on what it is */
    if (!strcmp(input_line, "uci")) {
        /* If the command sent was he "uci" command 
         * Boot the program up.
         *  -> Initialize zobrist hash keys
         *  -> Initialize magic bitboards
         *  -> Clear TP table.
        */

        // Data
        char program_name[256] = "The Cactus"; /* Name of the program */
        char program_author[256] = "Aaranyak Roy Ghosh"; /* Author */
        
        // Initialize pre-initialized data */
        init_tp_table();
        init_hash_keys();
        init_magic_tables();

        sprintf(output_line, "id name %s\nid author %s\nuciok\ninfo string The Cactus - A Chess Engine that is supposed to defeat humans in chess\n", program_name, program_author); /* Set the output of the engine */

        return; /* Just stop right there */

    } else if (!strcmp(input_line, "quit")) { /* GUI sends quit command */
        sprintf(output_line, ""); /* Don't print anything */
        *engine_running = 0; /* Stop running the engine */
        return; /* Same */
    } else if (!strcmp(input_line, "d")) {
        /* Ok.. display command */
        render_board(board); /* Yes... Render the board */
    } else if (!strcmp(input_line, "ucinewgame")) {
        /* New game message... */
        sprintf(output_line, "info string So are you ready...\n"); /* Input message */
    }

    // Handle other commands differently.

    else { /* If it is another command */
        /* To process this, we do it word by word */

        int next_index = 0; /* The next char index */
        char command_word[256] = {0}; /* The command (First Word) */
        while (1) { /* Remove space error */
            if (input_line[next_index] != ' ') break;
            next_index++;
        } /* Perfect... */
        
        next_index = get_word(input_line, command_word, next_index); /* Get the first word */

        // Different commands processed differently

        if (!strcmp(command_word, "isready")) {
            /* If the command is isready */
            if (!*thinking) sprintf(output_line, "readyok\n"); /* Print readyok */
            else { /* Wait... */
                *ready_command_sent = 1;
                sprintf(output_line, ""); /* Not yet ready */
            } /* Thread Safety */
            return; /* Go no further */
        } else if (!strcmp(command_word, "position")) { /* Position command creates position */
            /* Ok.. This will be kinda hard */
            parse_position(board, input_line, output_line, engine_running, thinking, ready_command_sent, next_index); /* Parse the position command */
            sprintf(output_line, ""); /* Put nothing here */
        } else if (!strcmp(command_word, "go")) {
            /* If the command is "go" */
            parse_go(board, input_line, output_line, engine_running, thinking, ready_command_sent, next_index); /* Parse the go command */
            return;
        }
    }

    sprintf(output_line, ""); /* Just don't */
    return;
}


int uci_engine() {
    /* Launches the engine in UCI protocol */

    // UCI protocol
    
    /* This will basically be an input-output loop between the engine (the cactus) and a UCI-compatible GUI
     * Read more about the UCI protocol here - http://page.mi.fu-berlin.de/block/uci.htm
    */

    // Before starting the loop, declare all the data
    Bitboard board = {0}; /* This is the board that we shall use for the chess game */
    char *raw_input = 0; /* This will be the string containing the input with \n */
    char input_line[2048] = {0}; /* This is the final input */
    char output_line[2048] = {0}; /* Again, string containing output */
    int engine_running = 1; /* For stopping the infinite loop */
    size_t idontknowwhyweneedthis = 0; /* But we do... */
    
    int thinking = 0;
    int ready_command_sent = 0;
    FILE *log_file;
    while (engine_running) {
        /* This is the input output loop */
        
        // Let's hope this works.
        setbuf(stdin, NULL);
        setbuf(stdout, NULL);
        
        getline(&raw_input, &idontknowwhyweneedthis, stdin); /* This will read the input line from the console */
        sscanf(raw_input, "%2046[^\n]", input_line); /* Format it properly */
        parse_uci_command(&board, input_line, output_line, &engine_running, &thinking, &ready_command_sent); /* Parses the UCI command */
        printf("%s", output_line); /* Print the output */
    }
    return 0;
}
