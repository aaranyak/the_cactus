CC = gcc
NAME = cactus
CFLAGS = -pthread -I/usr/include/gtk-3.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/gtk-3.0 -I/usr/include/gio-unix-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/fribidi -I/usr/include/harfbuzz -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/x86_64-linux-gnu -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

LDFLAGS = -lm -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0

# Sources
MOVE_GEN_SOURCES = pawn_moves.c knight_moves.c king_moves.c rook_moves.c bishop_moves.c queen_moves.c castling_moves.c generate_moves.c # Move generation code

SOURCES = main.c bitboard_utils.c move_utils.c move_gen_utils.c init_magics.c make_move.c legality_test.c evaluation.c perft_test.c search.c quiescence.c move_ordering.c zobrist_hash.c tp_table.c gui_game.c killer_moves.c $(MOVE_GEN_SOURCES) # All source files

all: $(SOURCES)
	$(CC) -no-pie -Wno-format-overflow -Wno-deprecated-declarations $(CFLAGS) -o $(NAME) $(SOURCES) $(LDFLAGS)
