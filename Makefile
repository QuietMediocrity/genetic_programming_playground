PKGS=sdl2 SDL2_gfx
CFLAGS=-Wall -Werror -Wold-style-definition -ggdb -std=c11 -pedantic `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm
OUTPUT_DIR_PATH = "output"

debug: src/main.c src/style.h src/game.h src/game.c src/rendering.h src/rendering.c
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR_PATH)/debug src/gp.c $(LIBS)
