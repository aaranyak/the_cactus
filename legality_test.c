/* legality_test.c
 * Contains functions to check if a move is legal, and also to test if king is under check 
*/

#include <stdio.h>
#include <stdlib.h>
#include "bitboards.h"
#include "bitboard_utils.h"
#include "moves.h"
#include "move_utils.h"
#include "rook_moves.h"
#include "bishop_moves.h"
#include "queen_moves.h"
#include "move_gen_utils.h"
#include "lookup_tables.h"
#include "make_move.h"

U64 pawn_attack_mask(Bitboard *board, int side) {
    /* Generate all attacked squares of pawns, to check if king is attacked */
    U64 attack_mask = 0; /* retval (you know what I mean) */
    if (side) { /* White pawn attacks */
        attack_mask = ((board->pieces[pawn_w] << 7) & ~files[7]) /* Move pawns up and left, and block out h-file */ 
                    | ((board->pieces[pawn_w] << 9) & ~files[0]); /* Move pawns up and right, and block out a-file */
    } else { /* Black pawn attacks */
        attack_mask = ((board->pieces[pawn_b] >> 9) & ~files[7]) /* Move pawns down and left, block out h-file */
                    | ((board->pieces[pawn_b] >> 7) & ~files[0]); /* Move pawns down and right a-file */
    }
    return attack_mask;
}

U64 knight_attack_mask(Bitboard *board, int side) {
    /* Return a bitmask for all the knight attacks for all the knights on the board */
    U64 knights = (side) ? board->pieces[knight_w] : board->pieces[knight_b]; /* Get all the knights we need (I'm sorry, I would like the comments to be a little less self-descriptive since I'm bored) */
    U64 attacks = 0; /* Probably got something to do with attacks */
    U64 position; /* Yeah we don't need to go over this again */
    int knight_index; /* Ditto */

    while (knights) { /* If you don't understand what i'm doing, go to a move-generation code file. */
        position = knights & -knights; /* Isolate LSB (I won't tell you why) */
        knight_index = bitscan(position); /* I cannot put a wierd comment on this line since what it does should be too obvious */
        attacks |= knight_attacks[knight_index]; /* lookup_tables.h */
        knights ^= position; /* Reset LSB (figure out what this means) */
    }
    return attacks; /* return the attacks */
}

U64 king_attack_mask(Bitboard *board, int side) {
    /* Ditto for king (but without the loop) */
    int king_index = bitscan(board->pieces[(side) ? king_w : king_b]); /* Get the index of the king */
    return king_attacks[king_index]; /* Because there is only ever one king (and he never dies) */
}

U64 rook_attack_mask(Bitboard *board, int side, U64 own, U64 enemy) {
    /* Get a bitmask of all the rook attacks */
    U64 rooks = board->pieces[side ? rook_w : rook_b]; /* Get the rooks of a certion relevant colour */
    U64 attacks = 0; /* The final bitmask of rook attacks to return */
    U64 position; /* The current rook position */
    int rook_index; /* Bitindex of current rook */

    while (rooks) { /* Loop through all the rooks */
        position = rooks & -rooks; /* Get the next rook (By getting the least significant bit of the rook bitboard) */
        rook_index = bitscan(position); /* Get the index of the rook */
        attacks |= magic_rook_moves(rook_index, own, enemy); /* Put rook attacks in the final mask */
        rooks ^= position; /* Move on to next rook (By removing this one) */
    }
    return attacks; /* Don't forget to add this line, or else all your effort will go to waste */
}

U64 bishop_attack_mask(Bitboard *board, int side, U64 own, U64 enemy) {
    /* Get a bitmask of all the bishop attacks */
    U64 bishops = board->pieces[side ? bishop_w : bishop_b]; /* Get the bishops of a certion relevant colour */
    U64 attacks = 0; /* The final bitmask of bishop attacks to return */
    U64 position; /* The current bishop position */
    int bishop_index; /* Bitindex of current bishop */

    while (bishops) { /* Loop through all the bishops */
        position = bishops & -bishops; /* Get the next bishop (By getting the least significant bit of the bishop bitboard) */
        bishop_index = bitscan(position); /* Get the index of the bishop */
        attacks |= magic_bishop_moves(bishop_index, own, enemy); /* Put bishop attacks in the final mask */
        bishops ^= position; /* Move on to next bishop (By removing this one) */
    }
    return attacks; /* Don't forget to add this line, or else all your effort will go to waste */
}

U64 queen_attack_mask(Bitboard *board, int side, U64 own, U64 enemy) {
    /* Get a bitmask of all the queen attacks */
    U64 queens = board->pieces[side ? queen_w : queen_b]; /* Get the queens of a certion relevant colour */
    U64 attacks = 0; /* The final bitmask of queen attacks to return */
    U64 position; /* The current queen position */
    int queen_index; /* Bitindex of current queen */

    while (queens) { /* Loop through all the queens */
        position = queens & -queens; /* Get the next queen (By getting the least significant bit of the queen bitboard) */
        queen_index = bitscan(position); /* Get the index of the queen */
        attacks |= magic_queen_moves(queen_index, own, enemy); /* Put queen attacks in the final mask */
        queens ^= position; /* Move on to next queen (By removing this one) */
    }
    return attacks; /* Don't forget to add this line, or else all your effort will go to waste */
}

int is_check(Bitboard *board, int side) {
    /* Detects if the king of any colour is under check */
    U64 own = colour_mask(board, side); /* Own colour mask */
    U64 enemy = colour_mask(board, !side); /* Enemy colour mask */
    U64 attacks = 0
        | pawn_attack_mask(board, !side) /* Add the pawn attacks */
        | knight_attack_mask(board, !side) /* Knight attacks */
        | king_attack_mask(board, !side) /* Ok, it seems obvious where we are going */
        | rook_attack_mask(board, !side, enemy, own)
        | bishop_attack_mask(board, !side, enemy, own)
        | queen_attack_mask(board, !side, enemy, own); /* Ok now we have got all the attacked squares by the enemy */
    // Check if the king intersects with the attacks
    if (board->pieces[side ? king_w : king_b] & attacks) return 1; /* Check! */
    else return 0; /* Not check */
}

int castling_legality(Bitboard *board, move_t move) {
    /* Special legality test for castling */
    int side = board->side;
    U64 own = colour_mask(board, side); /* Own colour mask */
    U64 enemy = colour_mask(board, !side); /* Enemy colour mask */
    int legality = 1;
    U64 attacks = 0
        | pawn_attack_mask(board, !side) /* Add the pawn attacks */
        | knight_attack_mask(board, !side) /* Knight attacks */
        | king_attack_mask(board, !side) /* Ok, it seems obvious where we are going */
        | rook_attack_mask(board, !side, enemy, own)
        | bishop_attack_mask(board, !side, enemy, own)
        | queen_attack_mask(board, !side, enemy, own); /* Ok now we have got all the attacked squares by the enemy */
    if (is_check(board, board->side)) legality = 0; /* Castling while checked is not allowed */
    U64 king_jumpover; /* The square that the king jumps over */
    if (board->side) { /* White castles */
        if (move & MM_CSD) king_jumpover = 8;
        else king_jumpover = 32;
    } else { /* If black is playing */
        if (move & MM_CSD) king_jumpover = 0x0800000000000000;
        else king_jumpover = 0x2000000000000000;
    }
    if (attacks & king_jumpover) legality = 0; /* King jumps over attacked square (Illegal!!) */
    return legality;
}

int is_legal(Bitboard *board, move_t move) {
    /* Return true if the move is legal, otherwise return false */
    U64 enpas, castling_rights, key; /* For unmaking move */
    int legality;
    make_move(board, move, &enpas, &castling_rights, &key); /* We Make the Move !! */
    legality = !is_check(board, !(board->side)); /* Check if the king is now under check (What if the king isn't even there? Don't think that's possible) */
    unmake_move(board, move, &enpas, &castling_rights, &key); /* We take back the move */
    if (move & MM_CAS) /* If this is a castling move */ legality = legality && castling_legality(board, move); /* Do special legality test */
    return legality;
}
