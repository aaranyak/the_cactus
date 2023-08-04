CC = gcc
CC = gcc
NAME = cactus
CFLAGS = 
LDFLAGS = -lm

# Sources

SOURCES = main.c bitboard_utils.c # All source files

all: $(SOURCES)
	$(CC) -no-pie -Wno-deprecated-declarations $(CFLAGS) -o $(NAME) $(SOURCES) $(LDFLAGS)
