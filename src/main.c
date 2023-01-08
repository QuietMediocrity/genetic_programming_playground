#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>

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

#define AGENTS_COUNT 32
#define FOOD_COUNT 128
#define WALLS_COUNT 128

static_assert(AGENTS_COUNT + FOOD_COUNT + WALLS_COUNT <= BOARD_WIDTH * BOARD_HEIGHT,
	      "Too many entities. You won't be able to fit all of them on game board.");

#define GENES_COUNT 24

#define FOOD_HUNGER_RECOVERY 10
#define ATTACK_DMG 10
#define RETALIATION_DMG 5
#define STARTING_HEALTH 100
#define STARTING_HUNGER 50
#define LETHAL_HUNGER 100
#define HUNGER_TICK 5

#define HEX_COLOR(hex_color)                                                                               \
	((hex_color) >> (2 * 8)) & 0xFF, ((hex_color) >> (1 * 8)) & 0xFF, ((hex_color) >> (0 * 8)) & 0xFF, \
		((hex_color) >> (3 * 8)) & 0xFF

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
} Direction;

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

// state | environment | action | next_state

typedef int AgentState;

typedef enum {
	ENV_NOTHING = 0,
	ENV_AGENT,
	ENV_FOOD,
	ENV_WALL,
	ENV_COUNT,
} Environment;

typedef enum {
	AA_NOTHING = 0,
	AA_STEP,
	AA_EAT,
	AA_ATTACK,
	AA_TURN_LEFT,
	AA_TURN_RIGHT,
	AA_COUNT,
} AgentAction;

typedef struct {
	AgentState current_state;
	AgentState next_state;
	Environment environment;
	AgentAction action;
} Gene;

typedef struct {
	size_t count;
	Gene genes[AGENTS_COUNT];
} Chromosome;

typedef struct {
	int x;
	int y;
} Position;

Position position_directions[4] = {
	{ 1, 0 }, // DIR_RIGHT
	{ 0, -1 }, // DIR_UP
	{ -1, 0 }, // DIR_LEFT
	{ 0, 1 }, // DIR_DOWN
};

typedef struct {
	Position pos;
	Direction direction;
	AgentState current_state;
	int hunger;
	int health;
} Agent;

typedef struct {
	int quantity;
	Position pos;
} Food;

typedef struct {
	Position pos;
} Wall;

typedef struct {
	Agent agents[AGENTS_COUNT];
	Chromosome chromosomes[AGENTS_COUNT];
	Food food[FOOD_COUNT];
	Wall walls[WALLS_COUNT];
} Game;

bool positions_are_equal(Position first, Position second) {
	return first.x == second.x && first.y == second.y;
}

bool is_cell_empty(const Game *game, Position pos) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		if (positions_are_equal(game->agents[i].pos, pos)) {
			return false;
		}
	}
	for (size_t i = 0; i < FOOD_COUNT; ++i) {
		if (positions_are_equal(game->food[i].pos, pos)) {
			return false;
		}
	}
	for (size_t i = 0; i < WALLS_COUNT; ++i) {
		if (positions_are_equal(game->walls[i].pos, pos)) {
			return false;
		}
	}

	return true;
}

int random_int_range(int low, int high) {
	return rand() % (high - low) + low;
}

Direction random_direction(void) {
	return (Direction)random_int_range(0, 4);
}

Position random_position(void) {
	Position result = { random_int_range(0, BOARD_WIDTH), random_int_range(0, BOARD_HEIGHT) };

	return result;
}

Position random_empty_position(const Game *game) {
	Position result = random_position();
	size_t it = 0;
	const size_t MAX_IT = 250;

	while (!is_cell_empty(game, result) && it < MAX_IT) {
		result = random_position();
	}

	assert(it < MAX_IT);
	return result;
}

Environment random_environment(void) {
	return random_int_range(0, ENV_COUNT);
}

AgentAction random_action(void) {
	return random_int_range(0, AA_COUNT);
}

void initialize_game(Game *game) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		game->agents[i].pos = random_empty_position(game);
		game->agents[i].direction = random_direction();
		game->agents[i].hunger = STARTING_HUNGER;
		game->agents[i].health = STARTING_HEALTH;

		// qm_todo: Remove this later.
		game->agents[i].direction = i % 4;

		for (size_t j = 0; j < GENES_COUNT; ++j) {
			game->chromosomes[i].genes[j].current_state = random_int_range(0, GENES_COUNT);
			game->chromosomes[i].genes[j].environment = random_environment();
			game->chromosomes[i].genes[j].action = random_action();
			game->chromosomes[i].genes[j].next_state = random_int_range(0, GENES_COUNT);
		}
	}

	// qm_todo: Yes, they can happen to be on top of each other, but who cares.
	// Maybe I'll fix it later.
	for (size_t i = 0; i < FOOD_COUNT; ++i) {
		game->food[i].pos = random_empty_position(game);
	}

	for (size_t i = 0; i < WALLS_COUNT; ++i) {
		game->walls[i].pos = random_empty_position(game);
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

int mod_int(int first, int second) {
	return (first % second + second) % second;
}

Position get_position_infront_of_agent(const Agent *agent) {
	Position delta = position_directions[agent->direction];
	Position next = agent->pos;

	next.x += mod_int(next.x + delta.x, BOARD_WIDTH);
	next.y += mod_int(next.y + delta.y, BOARD_HEIGHT);

	return next;
}

void move_agent(Agent *agent) {
	Position delta = position_directions[agent->direction];

	agent->pos.x = mod_int(agent->pos.x + delta.x, BOARD_WIDTH);
	agent->pos.y = mod_int(agent->pos.y + delta.y, BOARD_HEIGHT);
}

Food *get_ptr_to_food_infront_of_agent(Game *game, Agent *agent) {
	Position infront = get_position_infront_of_agent(agent);

	for (size_t i = 0; i < FOOD_COUNT; ++i) {
		if (game->food[i].quantity > 0 && positions_are_equal(game->food[i].pos, infront))
			return &game->food[i];
	}

	return NULL;
}

Agent *get_ptr_to_agent_infront_of_agent(Game *game, Agent *agent) {
	Position infront = get_position_infront_of_agent(agent);

	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		// qm_todo: do I need to check if agent[i] == *agent?
		// when will it be the case?
		if (positions_are_equal(game->agents[i].pos, infront) && game->agents[i].health > 0)
			return &game->agents[i];
	}

	return NULL;
}

Wall *get_ptr_to_wall_infront_of_agent(Game *game, Agent *agent) {
	Position infront = get_position_infront_of_agent(agent);

	for (size_t i = 0; i < WALLS_COUNT; ++i) {
		if (positions_are_equal(game->walls[i].pos, infront))
			return &game->walls[i];
	}

	return NULL;
}

Environment interpret_environment_infront_of_agent(Game *game, size_t agent_index) {
	Agent *agent = &game->agents[agent_index];

	// This order kind of serves as priority list.

	if (get_ptr_to_food_infront_of_agent(game, agent) != NULL)
		return ENV_FOOD;

	if (get_ptr_to_agent_infront_of_agent(game, agent) != NULL)
		return ENV_AGENT;

	if (get_ptr_to_wall_infront_of_agent(game, agent) != NULL)
		return ENV_WALL;

	return ENV_NOTHING;
}

void execute_action(Game *game, size_t agent_index, AgentAction action) {
	Agent *agent = &game->agents[agent_index];

	switch (action) {
	case AA_NOTHING:
		break;

	case AA_STEP:
		if (interpret_environment_infront_of_agent(game, agent_index) != ENV_WALL) {
			move_agent(agent);
		}
		break;

	case AA_EAT: {
		Food *food = get_ptr_to_food_infront_of_agent(game, agent);

		if (food != NULL) {
			food->quantity -= 1;
			agent->hunger -= FOOD_HUNGER_RECOVERY;

			if (agent->hunger < 0)
				agent->hunger = 0;
		}
	} break;

	case AA_ATTACK: {
		Agent *victim = get_ptr_to_agent_infront_of_agent(game, agent);

		if (victim != NULL) {
			victim->health -= ATTACK_DMG;
			agent->health -= RETALIATION_DMG;

			// No check for negative hp here.
			// We perform all actions first, then declare dead agents.
		}
	} break;

	case AA_TURN_LEFT:
		// this is absolutely brilliant!
		agent->direction = mod_int(agent->direction + 1, 4);
		break;

	case AA_TURN_RIGHT:
		agent->direction = mod_int(agent->direction - 1, 4);
		break;

	case AA_COUNT:
	default:
		assert(0 && "This is not supposed to happen, fix the 'action' value.");
		break;
	}
}

void game_step(Game *game) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		for (size_t j = 0; j < AGENTS_COUNT; ++j) {
			Gene *gene = &game->chromosomes[i].genes[j];

			if (gene->current_state != game->agents[i].current_state)
				continue;

			if (gene->environment != interpret_environment_infront_of_agent(game, i))
				continue;

			if (game->agents[i].health == 0)
				continue;

			execute_action(game, i, gene->action);
			game->agents[i].current_state = gene->next_state;
		}
	}

	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		if (game->agents[i].hunger == LETHAL_HUNGER) {
			game->agents[i].health -= HUNGER_TICK;
			continue;
		}

		game->agents[i].hunger += HUNGER_TICK;
	}
}

int main(int argc, char *argv[]) {
	srand(time(0));
	scc(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window *window =
		SDL_CreateWindow("QM's playground", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
	scp(window);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	scp(renderer);
	// scc(SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT));

	Game game;
	initialize_game(&game);

	int quit = 0;
	while (!quit) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT: {
				quit = 1;
			} break;
			case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
				case SDLK_r: {
					initialize_game(&game);
				} break;
				case SDLK_s: { // qm_todo: later on, make the ticks automatic after delta t, not manual.
					game_step(&game);
				} break;
				}
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
