/* move_utils.c
 * Includes functions to
 *  -> Create move
 *  -> Get move data
 *  -> Add move to move list
 *  -> Make move
 *  -> etc.
*/

#include <stdio.h>
#include "bitboards.h"
#include "moves.h"

move_t set_move(
        uint8_t from,
        uint8_t to,
        uint8_t piece,
        uint8_t captured
        )
{
    /* Creates a new move with the given parameters */
    move_t new_move = from; /* Create new move with the from */
    // Set other parameters
    new_move |= (move_t)to << MS_TO; /* Add shift the parameter to place and add it to the move */
    new_move |= (move_t)piece << MS_PIECE; /* Ditto */
    new_move |= (move_t)captured << MS_EAT;

    return new_move;
}

void add_move_to_list(move_list_t *move_list, move_t move) {
    /* Adds a new move to a move list */
    move_list->moves[move_list->count] = move; /* Add the move to the list's array */
    move_list->count++; /* Increment count */
}

void get_position_name(int position, char name[3]) { /* Get the name of the board position */
    /* Get the position name eg. e3 */
    char rn[8] = "12345678";
    char fn[8] = "abcdefgh";
    name[1] = rn[position / 8];
    name[0] = fn[position % 8];
    name[2] = 0;
}

void print_move(move_t move) {
    /* Prints a move to the console */
    char pieces[12][7] = {"rook", "knight", "bishop", "queen", "king", "pawn", "rook", "knight", "bishop", "queen", "king", "pawn"}; /* Piece id index */
    char from[3]; /* The from square */
    char to[3]; /* The to square */
    get_position_name((move & MM_FROM) >> MS_FROM, from); /* Get the position name */
    get_position_name((move & MM_TO) >> MS_TO, to); /* Get the position name */
    if (move & MM_CSD) printf("Castle on the Queen's side\n");
    else if (move & MM_CAS) printf("Castle on the King's side\n");
    else if (move & MM_EPC) printf("Move %s from %s to %s and make an en passant capture.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to);
    else if ((move & MM_CAP) && (move & MM_PRO)) printf("Move %s from %s to %s, capturing %s and promoting to %s.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to, pieces[(move & MM_EAT) >> MS_EAT], pieces[(move & MM_PPP) >> MS_PPP]);
    else if (move & MM_PRO) printf("Move %s from %s to %s and promote to %s.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to, pieces[(move & MM_PPP) >> MS_PPP]);

    else if (move & MM_CAP) printf("Move %s from %s to %s, capturing %s.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to, pieces[(move & MM_EAT) >> MS_EAT]);
    else printf("Move %s from %s to %s.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to);
}


void move_name(move_t move, char *enter) {
    /* Get move name */
    char pieces[12][7] = {"rook", "knight", "bishop", "queen", "king", "pawn", "rook", "knight", "bishop", "queen", "king", "pawn"}; /* Piece id index */
    char from[3]; /* The from square */
    char to[3]; /* The to square */
    get_position_name((move & MM_FROM) >> MS_FROM, from); /* Get the position name */
    get_position_name((move & MM_TO) >> MS_TO, to); /* Get the position name */
    if (move & MM_CSD) sprintf(enter, "Castle on the Queen's side\n");
    else if (move & MM_CAS) sprintf(enter, "Castle on the King's side\n");
    else if (move & MM_EPC) sprintf(enter, "Move %s from %s to %s and make an en passant capture.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to);
    else if ((move & MM_CAP) && (move & MM_PRO)) sprintf(enter, "Move %s from %s to %s, capturing %s and promoting to %s.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to, pieces[(move & MM_EAT) >> MS_EAT], pieces[(move & MM_PPP) >> MS_PPP]);
    else if (move & MM_PRO) sprintf(enter, "Move %s from %s to %s and promote to %s.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to, pieces[(move & MM_PPP) >> MS_PPP]);

    else if (move & MM_CAP) sprintf(enter, "Move %s from %s to %s, capturing %s.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to, pieces[(move & MM_EAT) >> MS_EAT]);
    else sprintf(enter, "Move %s from %s to %s.\n", pieces[(move & MM_PIECE) >> MS_PIECE], from, to);
}
