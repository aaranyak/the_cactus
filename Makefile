CC = gcc
CC = gcc
NAME = cactus
CFLAGS = 
LDFLAGS = -lm

# Sources
MOVE_GEN_SOURCES = pawn_moves.c # Move generation code

SOURCES = main.c bitboard_utils.c move_utils.c move_gen_utils.c $(MOVE_GEN_SOURCES) # All source files

all: $(SOURCES)
	$(CC) -no-pie -Wno-format-overflow -Wno-deprecated-declarations $(CFLAGS) -o $(NAME) $(SOURCES) $(LDFLAGS)
