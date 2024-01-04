CC = /mingw64/bin/gcc
NAME = cactus.exe
CFLAGS = -O3 # Compiler Flags

LDFLAGS = -lm -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32  # Linker Flags

# Sources
MOVE_GEN_SOURCES = pawn_moves.c knight_moves.c king_moves.c rook_moves.c bishop_moves.c queen_moves.c castling_moves.c generate_moves.c # Move generation code

SOURCES = main.c bitboard_utils.c move_utils.c move_gen_utils.c init_magics.c make_move.c legality_test.c evaluation.c perft_test.c search.c quiescence.c move_ordering.c zobrist_hash.c tp_table.c gui_game.c $(MOVE_GEN_SOURCES) # All source files

all: $(SOURCES)
	$(CC) -no-pie -Wno-format-overflow -Wno-deprecated-declarations $(CFLAGS) -o $(NAME) $(SOURCES) $(LDFLAGS)
