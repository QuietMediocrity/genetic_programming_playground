#ifndef GAME_H
#define GAME_H

#define BOARD_WIDTH 48 
#define BOARD_HEIGHT 25

#define AGENTS_COUNT 32
#define FOOD_COUNT 128
#define WALLS_COUNT 128
#define GENES_COUNT 24

#define FOOD_HUNGER_RECOVERY 10
#define ATTACK_DMG 10
#define RETALIATION_DMG 5
#define STARTING_HEALTH 100
#define STARTING_HUNGER 50
#define LETHAL_HUNGER 100
#define HUNGER_TICK 5

#include <stddef.h>
#include <stdio.h>

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
	Gene genes[GENES_COUNT];
} Chromosome;

typedef struct {
	int x;
	int y;
} Position;

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

void print_gene(FILE *stream, const Gene *gene, size_t agent_index, size_t gene_index);
void print_chromosome(FILE *stream, const Chromosome *chromosome, size_t agent_index);
void print_agent(FILE *stream, const Agent *a, size_t agent_index);

void initialize_game(Game *game);
void game_step(Game *game);

#endif // GAME_H
