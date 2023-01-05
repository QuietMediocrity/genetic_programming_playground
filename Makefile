PKGS=sdl2 SDL2_gfx
CFLAGS=-Wall -Wold-style-definition -ggdb -std=c11 -pedantic `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm
OUTPUT_DIR_PATH = "output"

gp: src/main.c
	$(CC) $(CFLAGS) -o $(OUTPUT_DIR_PATH)/gp src/main.c $(LIBS)
