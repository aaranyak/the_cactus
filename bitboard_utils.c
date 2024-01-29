/* bitboard_utils.c
 * Contains functions to 
 *  -> Initialize bitboard state from a char[]
 *  -> Render a bitboard stat on command line
 *  -> Parse FEN string.
*/
#include <stdio.h>
#include <stdlib.h>
#include "bitboards.h"
#include "math.h"
#include "legality_test.h"
#include "lookup_tables.h"
#include "zobrist_hash.h"

void clear_board(Bitboard *board) {
    /* Empties all the random data in a bitboard befor initiaizing it */
    for (int i = 0; i < 12; i++) { /* Loop through all piece bitboards */
        board->pieces[i] = 0; /* Empty them */
        board->attack_tables[i] = 0; /* Empty their attack tables as well */
    }
    for (int i = 0; i < 65536; i++) board->repetition_table[i] = 0; /* Clear repetition table */
    // Empty everything else
    board->castling_rights = 0;
    board->enpas = 0;
    board->side = 0;
    board->key = 0;
    board->moves = 0;
}

void init_board(Bitboard *board, char init_state[64], int side_to_move) {
    /* Initializes a board from an array of characters */
    clear_board(board); /* Reset board */
    U64 position; /* Index of... no not index. The actual square in bitboard */
    char letters[12] = "RNBQKPrnbqkp"; /* Letter names of pieces */
    char piece_type;
    for (int y = 0; y < 8; y++) { /* Loop through the rows in the array */ 
        for (int x = 0; x < 8; x++) { /* Loop through all the columns in a row */
            position = (U64)1 << ((7 - y) * 8) + x; /* Get actual position of the bit in the bitboard */
            piece_type = init_state[(y * 8) + x]; /* Get the piece type character */
            for (int p = 0; p < 12; p++) { /* loop through piece types */
                if (letters[p] == piece_type) {
                    board->pieces[p] |= position; /* Add piece to bb */
                    board->piece_square_eval += piece_square[p][((7 - y) * 8) + x];
                    // Update hash key
                    board->key ^= pst_hash[p][((7 - y) * 8) + x]; /* Xor the piece on square to the zobrist hash */
                }
            }
        }
    }
    // Set everything else
    board->castling_rights = W_CASTLE | B_CASTLE; /* Enable castling on both sides */
    board->side = side_to_move != 0;
    for (int piece = 0; piece < 12; piece++) { /* Loop through all piece types */
        update_attack_table(board, piece);
    }
    // Add the castling rights to the key
    board->key ^= cr_hash[0];
    board->key ^= cr_hash[1];
    board->key ^= cr_hash[2];
    board->key ^= cr_hash[4];
    if (side_to_move == 0) board->key != side_hash;
    board->repetition_table[0] = board->key;
}

void render_board(Bitboard *board) {
    /* Renders a bitboard to the terminal */
    char board_print[768]; /* The string to print out the board */
    char letters[12] = "RNBQKPrnbqkp"; /* The letter names of the pieces */
    char current_letter; /* Exactly what it means */
    U64 position;
    sprintf(board_print, "     a   b   c   d   e   f   g   h  \n   +---+---+---+---+---+---+---+---+\n"); /* Print first line */
    for (int y = 7; y >= 0; y--) { /* Loop through rows (backward) */
        sprintf(board_print, "%s %d |", board_print /* After everything */, y + 1); /* Print the first number */
        for (int x = 0; x < 8; x++) { /* Columns */
            position = 1;
            position <<= ((y * 8) + x); /* Bit that we are interested in */
            current_letter = ' '; /* Current letter */
            for (int p = 0; p < 12; p++) { /* Loop through piece types */
                if (position & board->pieces[p]) current_letter = letters[p]; /* Set the piece type */
            }
            sprintf(board_print, "%s %c |", board_print, current_letter); /* Blank square (just for now) */
        }
        sprintf(board_print, "%s\n   +---+---+---+---+---+---+---+---+\n", board_print); /* Next line */
    }
    printf("%sSide to move - %s\nPiece Square Eval - %d\nMoves - %d\nKey - 0x%016lx\n\n", board_print, board->side ? "White" : "Black", board->piece_square_eval, board->moves, board->key); /* print board */
}

void parse_fen(Bitboard *board, char *fen) {
    /* Parses a FEN string and gives a board */
    char init_state[64] = {
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',
    };
    int index = 0;
    int fen_index = 0;
    char current_letter;
    while (1) { /* Until broken */
        current_letter = fen[fen_index];
        if (current_letter == ' ') break;
        else if (current_letter == '/');
        else if (current_letter >= '0' && current_letter <= '8') {
            index += (current_letter - '0');
        }
        else {
            init_state[index] = current_letter;
            index++;
        }
        fen_index++;
    }
    fen_index++;
    char side_indicator = fen[fen_index];
    int side_to_move = side_indicator == 'w';
    fen_index += 2;
    init_board(board, init_state, side_to_move);
    board->castling_rights = 0;
    while (1) {
        current_letter = fen[fen_index];
        if (current_letter == ' ' || current_letter == '-') break;
        else {
            switch (current_letter) {
                case 'K':
                    board->castling_rights |= WK_CASTLE;
                    break;
                case 'Q':
                    board->castling_rights |= WQ_CASTLE;
                    break;
                case 'k':
                    board->castling_rights |= BK_CASTLE;
                    break;
                case 'q':
                    board->castling_rights |= BQ_CASTLE;
            }
            fen_index++;
        }
    }
    fen_index++;
    int ep_file = fen[fen_index] - 'a';
    if (ep_file >= 0 && ep_file < 8) board->enpas = files[ep_file];
    if (board->enpas) fen_index++;
    fen_index++;
    fen_index++;
    
    while (fen[fen_index] > '0' && fen[fen_index] < '9') fen_index++; /* Skip over the halfmove clock */

    fen_index++;
    fen_index++;

    int num_string_index = 0;
    int move_count = 0;
    while (fen[fen_index] > '0' && fen[fen_index] < '9') { /* Get the fullmove counter */
        move_count += (fen[fen_index] - '0') * pow(10, num_string_index); 
        fen_index++;
        num_string_index++;
    }
    move_count *= 2;
    move_count += board->side ? 0 : 1;
    
    board->moves = move_count;
    
}
