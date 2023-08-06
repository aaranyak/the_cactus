/* moves.h
 * Stores type definitions and macros for move encoding and move list.
*/
#ifndef MOVES_H /* Header Guard */
#define MOVES_H

// Move encoding system

typedef unsigned int move; /* a move is basically an unsigned int, with parts of it being the data required */
// Move encoding
/* First byte - Square from
 * Second byte - Square to
 * Next four bits - Piece to move
 * Next four bits - Captured piece (if any)
 * Last eight bits - Bit Flags // Figure this out later
*/

// Bitmasks for the move
#define MM_FROM 0xff /* Square from bitmask */
#define MM_TO 0xff00 /* Square to bitmask */
#define MM_PIECE 0x0f0000 /* Piece to move bitmask */
#define MM_EAT 0xf00000 /* Captured piece id bitmask */
#define MM_FLAGS 0xff000000 /* Move flags bitmask */

// Bitshifts for the move (>>)
#define MS_FROM 0 /* Has no bitshift, since is at the beginning */
#define MS_TO 8 /* Shift secnond byte to to beginning */
#define MS_PIECE 16 /* Bitshift for piece to move id */
#define MS_EAT 20 /* Bitshift for captured piece id */
#define MS_FLAGS 24 /* Bitshift for flags */ /* Is this really necessary, or can we just look up flags directly */

// Move list system
typedef struct move_list {
    move moves[256]; /* An array of moves, large enough to fit all */
    int count; /* Number of moves in list */
}
#endif
