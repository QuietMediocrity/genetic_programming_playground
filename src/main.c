#include <SDL2/SDL_render.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL2/SDL.h>

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

Uint8 hex_to_decimal(char c) {
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10;

	fprintf(stderr, "ERROR: Incorrect hex character provided: %c\n", c);
	exit(1);
}

Uint8 parse_hex_byte(const char *hex_byte) {
	return hex_to_decimal(*hex_byte) * 0x10 + hex_to_decimal(*(hex_byte + 1));
}

void sdl_set_hex_color_to_draw_line(SDL_Renderer *renderer, const char *hex_color) {
	assert(strlen(hex_color) == 6);
	scc(SDL_SetRenderDrawColor(renderer,
				   parse_hex_byte(hex_color + 0),
				   parse_hex_byte(hex_color + 2),
				   parse_hex_byte(hex_color + 4),
				   255));
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
	}
}

void render_agent(SDL_Renderer *renderer, size_t index) {
	sdl_set_hex_color_to_draw_line(renderer, AGENT_COLOR);
	agent *a = &agents[index];

	SDL_Rect rect = {
		(int)floorf(a->pos_x * CELL_WIDTH),
		(int)floorf(a->pos_y * CELL_HEIGHT),
		(int)floorf(CELL_WIDTH),
		(int)floorf(CELL_HEIGHT),
	};

	scc(SDL_RenderFillRect(renderer, &rect));
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
