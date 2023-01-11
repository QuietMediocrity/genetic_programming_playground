#ifndef GAME_H
#define GAME_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#define BOARD_WIDTH 48
#define BOARD_HEIGHT 25

#define AGENTS_COUNT 128
#define FOOD_COUNT 256
#define WALLS_COUNT 64
#define GENES_COUNT 128
#define STATES_COUNT 8

#define FOOD_HUNGER_RECOVERY 30
#define FOOD_QUANTITY_GENERATION_MAX 4
#define ATTACK_DMG 10
#define RETALIATION_DMG 5
#define STARTING_HEALTH 100
#define STARTING_HUNGER 50
#define LETHAL_HUNGER 100
#define MAX_LIFETIME 512
#define HUNGER_TICK 5

#define MUTATION_PROBABILITY 256
#define MUTATION_THRESHHOLD 16
#define MATING_SELECTION_POOL 16

typedef enum {
	DIR_RIGHT = 0,
	DIR_UP,
	DIR_LEFT,
	DIR_DOWN,
} Direction;

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
	AA_TURN_LEFT,
	AA_TURN_RIGHT,
	AA_COUNT,
} AgentAction;

typedef enum {
	VA_NOTHING = 0,
	VA_STEP,
	VA_FOOD,
	VA_ATTACK,
	VA_TURN_LEFT,
	VA_TURN_RIGHT,
	VA_COUNT,
} VerboseAction;

typedef struct {
	AgentState current_state;
	AgentState next_state;
	Environment environment;
	AgentAction action;
} Gene;

typedef struct {
	size_t count;
	Gene genes[GENES_COUNT];
} Chromosome;

typedef struct {
	int x;
	int y;
} Position;

typedef struct {
	size_t index;
	Position pos;
	Direction direction;
	AgentState current_state;
	int hunger;
	int health;
	size_t lifetime;
	VerboseAction action_history[MAX_LIFETIME];
	int used_genes_history[MAX_LIFETIME];
	Chromosome chromosome;
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
	Food food[FOOD_COUNT];
	Wall walls[WALLS_COUNT];
} Game;

int mod_int(int first, int second);

void print_gene(FILE *stream, const Gene *gene, size_t agent_index, size_t gene_index);
void print_chromosome(FILE *stream, const Chromosome *chromosome, size_t agent_index);
void print_agent(FILE *stream, const Agent *a);
void print_agent_verbose(FILE *stream, const Agent *a);
void print_the_state_of_oldest_agent(Game *game);

Position get_position_infront_of_agent(const Agent *agent);

Food *get_ptr_to_food_infront_of_agent(Game *game, Agent *agent);
Agent *get_ptr_to_agent_infront_of_agent(Game *game, Agent *agent);
Wall *get_ptr_to_wall_infront_of_agent(Game *game, Agent *agent);

Agent *get_ptr_to_agent_at_pos(Game *game, Position pos);
Food *get_ptr_to_food_at_pos(Game *game, Position pos);
Wall *get_ptr_to_wall_at_pos(Game *game, Position pos);

void initialize_game(Game *game);
void game_step(Game *game);
void prepare_next_game(Game *previous_game, Game *next_game);

void dump_game_state(const char *filepath, const Game *game);
void load_game_state(const char *filepath, Game *game);
bool is_everyone_dead(const Game *game);

#endif // GAME_H
