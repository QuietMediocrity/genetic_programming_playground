#include "rendering.h"

#include <stdio.h>

#define HEX_COLOR(hex_color)                                                                               \
	((hex_color) >> (2 * 8)) & 0xFF, ((hex_color) >> (1 * 8)) & 0xFF, ((hex_color) >> (0 * 8)) & 0xFF, \
		((hex_color) >> (3 * 8)) & 0xFF

void render_agent(SDL_Renderer *renderer, const Game *game, size_t index);

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
float agent_directions[4][6] = {
	{ 0.0, 0.0, 1.0, 0.5, 0.0, 1.0 }, // DIR_RIGHT
	{ 0.0, 1.0, 0.5, 0.0, 1.0, 1.0 }, // DIR_UP
	{ 1.0, 0.0, 1.0, 1.0, 0.0, 0.5 }, // DIR_LEFT
	{ 0.0, 0.0, 1.0, 0.0, 0.5, 1.0 }, // DIR_DOWN
};

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

void clear_board(SDL_Renderer *renderer)
{
	scc(SDL_SetRenderDrawColor(renderer, HEX_COLOR(BACKGROUND_COLOR)));
	scc(SDL_RenderClear(renderer));
}

void render_board_grid(SDL_Renderer *renderer) {
	scc(SDL_SetRenderDrawColor(renderer, HEX_COLOR(LINE_COLOR)));

	for (int x = 1; x < BOARD_WIDTH; ++x) {
		scc(SDL_RenderDrawLine(renderer, x * CELL_WIDTH, 0, x * CELL_WIDTH, SCREEN_HEIGHT));
	}
	for (int y = 1; y < BOARD_HEIGHT; ++y) {
		scc(SDL_RenderDrawLine(renderer, 0, y * CELL_HEIGHT, SCREEN_WIDTH, y * CELL_HEIGHT));
	}
}

void render_agent(SDL_Renderer *renderer, const Game *game, size_t index) {
	const float AGENT_PADDING = 6.f;
	const float CELL_WIDTH_PADDING = CELL_WIDTH - AGENT_PADDING * 2;
	const float CELL_HEIGHT_PADDING = CELL_HEIGHT - AGENT_PADDING * 2;
	const Agent *a = &game->agents[index];

	const float x1 = agent_directions[a->direction][0] * CELL_WIDTH_PADDING + a->pos.x * CELL_WIDTH + AGENT_PADDING;
	const float y1 =
		agent_directions[a->direction][1] * CELL_HEIGHT_PADDING + a->pos.y * CELL_HEIGHT + AGENT_PADDING;
	const float x2 = agent_directions[a->direction][2] * CELL_WIDTH_PADDING + a->pos.x * CELL_WIDTH + AGENT_PADDING;
	const float y2 =
		agent_directions[a->direction][3] * CELL_HEIGHT_PADDING + a->pos.y * CELL_HEIGHT + AGENT_PADDING;
	const float x3 = agent_directions[a->direction][4] * CELL_WIDTH_PADDING + a->pos.x * CELL_WIDTH + AGENT_PADDING;
	const float y3 =
		agent_directions[a->direction][5] * CELL_HEIGHT_PADDING + a->pos.y * CELL_HEIGHT + AGENT_PADDING;

	if (a->health > 0) {
		filledTrigonColor(renderer, x1, y1, x2, y2, x3, y3, AGENT_COLOR);
		aatrigonColor(renderer, x1, y1, x2, y2, x3, y3, AGENT_COLOR);
	} else {
		filledTrigonColor(renderer, x1, y1, x2, y2, x3, y3, DEAD_AGENT_COLOR);
		aatrigonColor(renderer, x1, y1, x2, y2, x3, y3, DEAD_AGENT_COLOR);
	}
}

void render_game(SDL_Renderer *renderer, const Game *game) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		render_agent(renderer, game, i);
	}

	const float FOOD_PADDING = 12.5f;
	for (size_t i = 0; i < FOOD_COUNT; ++i) {
		if (game->food[i].quantity == 0)
			continue;

		filledCircleRGBA(renderer,
				 (int)floorf(game->food[i].pos.x * CELL_WIDTH + CELL_WIDTH * 0.5f),
				 (int)floorf(game->food[i].pos.y * CELL_HEIGHT + CELL_HEIGHT * 0.5f),
				 (int)floorf(fminf(CELL_WIDTH, CELL_HEIGHT) * 0.5f - FOOD_PADDING),
				 HEX_COLOR(FOOD_COLOR));
	}

	// const float WALL_PADDING = 4.f;
	const float WALL_PADDING = 0.f;
	scc(SDL_SetRenderDrawColor(renderer, HEX_COLOR(WALL_COLOR)));
	for (size_t i = 0; i < WALLS_COUNT; ++i) {
		SDL_Rect rect = {
			(int)floorf(game->walls[i].pos.x * CELL_WIDTH + WALL_PADDING),
			(int)floorf(game->walls[i].pos.y * CELL_HEIGHT + WALL_PADDING),
			(int)floorf(CELL_WIDTH - 2 * WALL_PADDING),
			(int)floorf(CELL_HEIGHT - 2 * WALL_PADDING),
		};

		scc(SDL_RenderFillRect(renderer, &rect));
	}
}
