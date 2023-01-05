#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define BOARD_WIDTH 20
#define BOARD_HEIGHT 20

#define CELL_WIDTH ((float)SCREEN_WIDTH / BOARD_WIDTH)
#define CELL_HEIGHT ((float)SCREEN_HEIGHT / BOARD_HEIGHT)

#define AGENTS_COUNT 100

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

void render_board_grid(SDL_Renderer *renderer)
{
	scc(SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255));

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

int random_int_range(int low, int high)
{
	return rand() % (high - low) + low;
}

direction random_direction(void)
{
	return (direction)random_int_range(0, 4);
}

agent random_agent(void)
{
	agent a = { 0 };

	a.pos_x = random_int_range(0, BOARD_WIDTH);
	a.pos_y = random_int_range(0, BOARD_HEIGHT);
	a.direction = random_direction();
	a.hunder = 100;
	a.health = 100;

	return a;
}

void initialize_agents(void)
{
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		agents[i] = random_agent();
	}
}

int main(int argc, char *argv[])
{
	scc(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window *window =
		SDL_CreateWindow("QM's playground", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
	scp(window);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	scp(renderer);
	scc(SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT));

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

		scc(SDL_SetRenderDrawColor(renderer, 36, 41, 46, 255));
		scc(SDL_RenderClear(renderer));

		render_board_grid(renderer);

		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
	return 0;
}
