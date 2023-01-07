#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "style.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define BOARD_WIDTH 20
#define BOARD_HEIGHT 20

#define CELL_WIDTH ((float)SCREEN_WIDTH / BOARD_WIDTH)
#define CELL_HEIGHT ((float)SCREEN_HEIGHT / BOARD_HEIGHT)

#define AGENTS_COUNT 10

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

void sdl_set_hex_color_to_draw_line(SDL_Renderer *renderer, Uint32 hex_color) {
	scc(SDL_SetRenderDrawColor(renderer,
				   (hex_color >> (2 * 8)) & 0xFF,
				   (hex_color >> (1 * 8)) & 0xFF,
				   (hex_color >> (0 * 8)) & 0xFF,
				   (hex_color >> (3 * 8)) & 0xFF));
}

void render_board_grid(SDL_Renderer *renderer) {
	sdl_set_hex_color_to_draw_line(renderer, LINE_COLOR);

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

typedef struct {
	int pos_x, pos_y;
	direction direction;
	int hunder;
	int health;
} agent;

typedef enum {
	AA_NOTHING = 0,
	AA_STEP,
	AA_EAT,
	AA_ATTACK,
} agent_action;

agent agents[AGENTS_COUNT];

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

void initialize_agents(void) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		agents[i] = random_agent();

		// qm_todo: remove this later.
		agents[i].direction = i % 4;
	}
}

void render_agent(SDL_Renderer *renderer, size_t index) {
	const float AGENT_PADDING = 6.f;
	const float CELL_WIDTH_PADDING = CELL_WIDTH - AGENT_PADDING * 2;
	const float CELL_HEIGHT_PADDING = CELL_HEIGHT - AGENT_PADDING * 2;
	const agent *a = &agents[index];

	const float x1 = agents_dirs[a->direction][0] * CELL_WIDTH_PADDING + a->pos_x * CELL_WIDTH + AGENT_PADDING;
	const float y1 = agents_dirs[a->direction][1] * CELL_HEIGHT_PADDING + a->pos_y * CELL_HEIGHT + AGENT_PADDING;
	const float x2 = agents_dirs[a->direction][2] * CELL_WIDTH_PADDING + a->pos_x * CELL_WIDTH + AGENT_PADDING;
	const float y2 = agents_dirs[a->direction][3] * CELL_HEIGHT_PADDING + a->pos_y * CELL_HEIGHT + AGENT_PADDING;
	const float x3 = agents_dirs[a->direction][4] * CELL_WIDTH_PADDING + a->pos_x * CELL_WIDTH + AGENT_PADDING;
	const float y3 = agents_dirs[a->direction][5] * CELL_HEIGHT_PADDING + a->pos_y * CELL_HEIGHT + AGENT_PADDING;

	filledTrigonColor(renderer, x1, y1, x2, y2, x3, y3, AGENT_COLOR);
	aatrigonColor(renderer, x1, y1, x2, y2, x3, y3, AGENT_COLOR);
}

void render_agents(SDL_Renderer *renderer) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		render_agent(renderer, i);
	}
}

int main(int argc, char *argv[]) {
	scc(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window *window =
		SDL_CreateWindow("QM's playground", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
	scp(window);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	scp(renderer);
	scc(SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT));

	initialize_agents();

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

		sdl_set_hex_color_to_draw_line(renderer, BACKGROUND_COLOR);
		scc(SDL_RenderClear(renderer));

		render_board_grid(renderer);
		render_agents(renderer);

		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
	return 0;
}
