#ifndef RENDERING_H
#define RENDERING_H

#include "style.h"
#include "game.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1015 // apparently I am using 65px for OS header and program header in windowed mode.

#define CELL_WIDTH ((float)SCREEN_WIDTH / BOARD_WIDTH)
#define CELL_HEIGHT ((float)SCREEN_HEIGHT / BOARD_HEIGHT)

void scc(int code); // sdl check code
void scp(const void *ptr); // sdl check pointer

void clear_board(SDL_Renderer *renderer);
void render_game(SDL_Renderer *renderer, const Game *game);

#endif // !RENDERING_H
