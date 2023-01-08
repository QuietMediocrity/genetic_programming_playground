#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "style.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1015 // apparently I am using 65px for OS header and program header in windowed mode.

#define BOARD_WIDTH 48 // 1920 / 40 = 48 ?
// #define BOARD_HEIGHT 27 // 1080 / 40 = 27 ?
#define BOARD_HEIGHT 25 

#define CELL_WIDTH ((float)SCREEN_WIDTH / BOARD_WIDTH)
#define CELL_HEIGHT ((float)SCREEN_HEIGHT / BOARD_HEIGHT)

#define AGENTS_COUNT 10
#define FOOD_COUNT 15
#define WALL_COUNT 8

void scc(int code) // sdl check code
{
	if (code < 0) {
		fprintf(stderr, "SDL error: %s\n", SDL_GetError());
		exit(1);
	}
}

void scp(const void *ptr) // sdl check pointer
{
	if (ptr == NULL) {
		fprintf(stderr, "SDL error: %s\n", SDL_GetError());
		exit(1);
	}
}

#define HEX_COLOR(hex_color)             \
	((hex_color) >> (2 * 8)) & 0xFF, \
        ((hex_color) >> (1 * 8)) & 0xFF, \
	((hex_color) >> (0 * 8)) & 0xFF, \
	((hex_color) >> (3 * 8)) & 0xFF

void render_board_grid(SDL_Renderer *renderer) {
	scc(SDL_SetRenderDrawColor(renderer, HEX_COLOR(LINE_COLOR)));

	for (int x = 1; x < BOARD_WIDTH; ++x) {
		scc(SDL_RenderDrawLine(renderer, x * CELL_WIDTH, 0, x * CELL_WIDTH, SCREEN_HEIGHT));
	}
	for (int y = 1; y < BOARD_HEIGHT; ++y) {
		scc(SDL_RenderDrawLine(renderer, 0, y * CELL_HEIGHT, SCREEN_WIDTH, y * CELL_HEIGHT));
	}
}

typedef enum {
	DIR_RIGHT = 0,
	DIR_UP,
	DIR_LEFT,
	DIR_DOWN,
} direction;

/*
 * This is used to render pointy triangular agents,
 * where individual triangles are fit into 0-1 coordinate system
 *    0 - - - - -> 1
 *    | *
 *    | ****
 *    | *******
 *    | *********  <- 0.5
 *    | *******
 *    | ****
 *    | *
 *    1
 */
float agents_dirs[4][6] = {
	{ 0.0, 0.0, 1.0, 0.5, 0.0, 1.0 }, // DIR_RIGHT
	{ 0.0, 1.0, 0.5, 0.0, 1.0, 1.0 }, // DIR_UP
	{ 1.0, 0.0, 1.0, 1.0, 0.0, 0.5 }, // DIR_LEFT
	{ 0.0, 0.0, 1.0, 0.0, 0.5, 1.0 }, // DIR_DOWN
};

// state | environment | action | next_state

typedef int agent_state;

typedef enum { ENV_NOTHING = 0, ENV_AGENT, ENV_FOOD } environment;

typedef enum {
	AA_NOTHING = 0,
	AA_STEP,
	AA_EAT,
	AA_ATTACK,
} agent_action;

typedef struct {
	agent_state current_state;
	agent_state next_state;
	environment environment;
	agent_action action;
} brain_cell;

typedef struct {
	size_t count;
	brain_cell cells[];
} brain;

typedef struct {
	int pos_x, pos_y;
	direction direction;
	int hunder;
	int health;
} agent;

typedef struct {
	int quantity;
	int pos_x;
	int pos_y;
} food;

typedef struct {
	int pos_x;
	int pos_y;
} wall;

typedef struct {
	agent agents[AGENTS_COUNT];
	food food[FOOD_COUNT];
	wall walls[WALL_COUNT];
} game;

int random_int_range(int low, int high) {
	return rand() % (high - low) + low;
}

direction random_direction(void) {
	return (direction)random_int_range(0, 4);
}

agent random_agent(void) {
	agent a = { 0 };

	a.pos_x = random_int_range(0, BOARD_WIDTH);
	a.pos_y = random_int_range(0, BOARD_HEIGHT);
	a.direction = random_direction();
	a.hunder = 100;
	a.health = 100;

	return a;
}

void initialize_game(game *game) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		game->agents[i] = random_agent();

		// qm_todo: Remove this later.
		game->agents[i].direction = i % 4;
	}

	// qm_todo: Yes, they can happen to be on top of each other, but
	// who cares. Maybe I'll fix it later.
	for (size_t i = 0; i < FOOD_COUNT; ++i) {
		game->food[i].pos_x = random_int_range(0, BOARD_WIDTH);
		game->food[i].pos_y = random_int_range(0, BOARD_HEIGHT);
	}

	for (size_t i = 0; i < WALL_COUNT; ++i) {
		game->walls[i].pos_x = random_int_range(0, BOARD_WIDTH);
		game->walls[i].pos_y = random_int_range(0, BOARD_HEIGHT);
	}
}

void render_agent(SDL_Renderer *renderer, const game *game, size_t index) {
	const float AGENT_PADDING = 6.f;
	const float CELL_WIDTH_PADDING = CELL_WIDTH - AGENT_PADDING * 2;
	const float CELL_HEIGHT_PADDING = CELL_HEIGHT - AGENT_PADDING * 2;
	const agent *a = &game->agents[index];

	const float x1 = agents_dirs[a->direction][0] * CELL_WIDTH_PADDING + a->pos_x * CELL_WIDTH + AGENT_PADDING;
	const float y1 = agents_dirs[a->direction][1] * CELL_HEIGHT_PADDING + a->pos_y * CELL_HEIGHT + AGENT_PADDING;
	const float x2 = agents_dirs[a->direction][2] * CELL_WIDTH_PADDING + a->pos_x * CELL_WIDTH + AGENT_PADDING;
	const float y2 = agents_dirs[a->direction][3] * CELL_HEIGHT_PADDING + a->pos_y * CELL_HEIGHT + AGENT_PADDING;
	const float x3 = agents_dirs[a->direction][4] * CELL_WIDTH_PADDING + a->pos_x * CELL_WIDTH + AGENT_PADDING;
	const float y3 = agents_dirs[a->direction][5] * CELL_HEIGHT_PADDING + a->pos_y * CELL_HEIGHT + AGENT_PADDING;

	filledTrigonColor(renderer, x1, y1, x2, y2, x3, y3, AGENT_COLOR);
	aatrigonColor(renderer, x1, y1, x2, y2, x3, y3, AGENT_COLOR);
}

void render_game(SDL_Renderer *renderer, const game *game) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		render_agent(renderer, game, i);
	}

	const float FOOD_PADDING = 10.f;
	for (size_t i = 0; i < FOOD_COUNT; ++i) {
		filledCircleRGBA(renderer,
				 (int)floorf(game->food[i].pos_x * CELL_WIDTH + CELL_WIDTH * 0.5f),
				 (int)floorf(game->food[i].pos_y * CELL_HEIGHT + CELL_HEIGHT * 0.5f),
				 (int)floorf(fminf(CELL_WIDTH, CELL_HEIGHT) * 0.5f - FOOD_PADDING),
				 HEX_COLOR(FOOD_COLOR));
	}

	const float WALL_PADDING = 4.f;
	scc(SDL_SetRenderDrawColor(renderer, HEX_COLOR(WALL_COLOR)));
	for (size_t i = 0; i < WALL_COUNT; ++i) {
		SDL_Rect rect = {
			(int)floorf(game->walls[i].pos_x * CELL_WIDTH + WALL_PADDING),
			(int)floorf(game->walls[i].pos_y * CELL_HEIGHT + WALL_PADDING),
			(int)floorf(CELL_WIDTH - 2 * WALL_PADDING),
			(int)floorf(CELL_HEIGHT - 2 * WALL_PADDING),
		};

		scc(SDL_RenderFillRect(renderer, &rect));
	}
}

int main(int argc, char *argv[]) {
	scc(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window *window =
		SDL_CreateWindow("QM's playground", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
	scp(window);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	scp(renderer);
	// scc(SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT));

	game game;
	initialize_game(&game);

	int quit = 0;
	while (!quit) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT: {
				quit = 1;
			} break;
			}
		}

	        scc(SDL_SetRenderDrawColor(renderer, HEX_COLOR(BACKGROUND_COLOR)));
		scc(SDL_RenderClear(renderer));

		render_board_grid(renderer);
		render_game(renderer, &game);

		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
	return 0;
}
