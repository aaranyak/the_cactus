/* moves.h
 * Stores type definitions and macros for move encoding and move list.
*/
#ifndef MOVES_H /* Header Guard */
#define MOVES_H

// Move encoding system

typedef unsigned int move_t; /* a move is basically an unsigned int, with parts of it being the data required */
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
// Flags
#define MM_CAP 0x01000000 /* Capture flag bitmask */
#define MM_DPP 0x02000000 /* Flag for double pawn push */
#define MM_CAS 0x04000000 /* Castling flag bitmask */
#define MM_CSD 0x08000000 /* Side to castle flag */
#define MM_EPC 0x10000000 /* EP Capture Flag */
#define MM_PRO 0x20000000 /* Pawn promotion flag */
#define MM_PPP 0xc0000000 /* Promote to piece flag */
// Bitshifts for the move (>>)
#define MS_FROM 0 /* Has no bitshift, since is at the beginning */
#define MS_TO 8 /* Shift secnond byte to to beginning */
#define MS_PIECE 16 /* Bitshift for piece to move id */
#define MS_EAT 20 /* Bitshift for captured piece id */
// Flags
#define MS_PPP 30 /* Bitshift for promote to piece flag */

// Move list system
typedef struct move_list_t{
    move_t moves[256]; /* An array of moves, large enough to fit all */
    int count; /* Number of moves in list */
} move_list_t;
#endif
