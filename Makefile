CC = gcc
CC = gcc
NAME = cactus
CFLAGS = 
LDFLAGS = -lm

# Sources
MOVE_GEN_SOURCES = pawn_moves.c knight_moves.c king_moves.c rook_moves.c bishop_moves.c queen_moves.c castling_moves.c # Move generation code

SOURCES = main.c bitboard_utils.c move_utils.c move_gen_utils.c init_magics.c make_move.c $(MOVE_GEN_SOURCES) # All source files

all: $(SOURCES)
	$(CC) -no-pie -Wno-format-overflow -Wno-deprecated-declarations $(CFLAGS) -o $(NAME) $(SOURCES) $(LDFLAGS)
