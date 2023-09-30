CC = gcc
NAME = cactus
CFLAGS = 
LDFLAGS = -lm

# Sources
MOVE_GEN_SOURCES = pawn_moves.c knight_moves.c king_moves.c rook_moves.c bishop_moves.c queen_moves.c castling_moves.c generate_moves.c # Move generation code

SOURCES = main.c bitboard_utils.c move_utils.c move_gen_utils.c init_magics.c make_move.c legality_test.c evaluation.c perft_test.c search.c quiescence.c move_ordering.c zobrist_hash.c tp_table.c $(MOVE_GEN_SOURCES) # All source files

all: $(SOURCES)
	$(CC) -no-pie -Wno-format-overflow -Wno-deprecated-declarations $(CFLAGS) -o $(NAME) $(SOURCES) $(LDFLAGS)
