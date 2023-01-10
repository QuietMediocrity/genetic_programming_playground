#include "game.h"
#include "style.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static_assert(AGENTS_COUNT + FOOD_COUNT + WALLS_COUNT <= BOARD_WIDTH * BOARD_HEIGHT,
	      "Too many entities. You won't be able to fit all of them on game board.");
static_assert(GENES_COUNT % 2 == 0, "Genes count has to be an even number for proper work of evolution.");

Position position_directions[4] = {
	{ 1, 0 }, // DIR_RIGHT
	{ 0, -1 }, // DIR_UP
	{ -1, 0 }, // DIR_LEFT
	{ 0, 1 }, // DIR_DOWN
};

const char *env_as_cstr(Environment env);
const char *action_as_cstr(AgentAction a);
const char *direction_as_cstr(Direction d);

bool positions_are_equal(Position first, Position second);
bool is_cell_empty(const Game *game, Position pos);

int random_int_range(int low, int high);
Direction random_direction(void);
Position random_position(void);
Position random_empty_position(const Game *game);
Environment random_environment(void);
AgentAction random_action(void);

void initialize_basic_agent_properties(Game *game, Agent *agent, size_t agent_index);
void initialize_gene(Gene *gene);
void initialize_food(Game *game);
void initialize_walls(Game *game);

int mod_int(int first, int second);
void move_agent(Agent *agent);

Position get_position_infront_of_agent(const Agent *agent);
Food *get_ptr_to_food_infront_of_agent(Game *game, Agent *agent);
Agent *get_ptr_to_agent_infront_of_agent(Game *game, Agent *agent);
Wall *get_ptr_to_wall_infront_of_agent(Game *game, Agent *agent);

Environment interpret_environment_infront_of_agent(Game *game, Agent *agent);
void execute_action(Game *game, Agent *agent, AgentAction action);

void mate_agents(const Agent *parent_a, const Agent *parent_b, Agent *child);
void mutate_agent(Agent *agent);
int agent_lifetime_comparator(const void *a, const void *b);
void prepare_next_generation(Game *previous_game, Game *next_game);

void print_gene(FILE *stream, const Gene *gene, size_t agent_index, size_t gene_index) {
	fprintf(stream,
		"\t\tagent_index: %2zu    gene_index: %3zu    c_state: %3d    env: %15s    action: %15s    n_state: %3d\n",
		agent_index,
		gene_index,
		gene->current_state,
		env_as_cstr(gene->environment),
		action_as_cstr(gene->action),
		gene->next_state);
}

void print_chromosome(FILE *stream, const Chromosome *chromosome, size_t agent_index) {
	for (size_t i = 0; i < GENES_COUNT; ++i) {
		print_gene(stream, &chromosome->genes[i], agent_index, i);
	}
}

void print_agent(FILE *stream, const Agent *a) {
	fprintf(stream,
		"\nindex: %zu\tpos: [%d;%d]\tstate: %d\tdirection: %s\thunger: %d\thealth: %d\t",
		a->index,
		a->pos.x,
		a->pos.y,
		a->current_state,
		direction_as_cstr(a->direction),
		a->hunger,
		a->health);
}

void print_agent_verbose(FILE *stream, const Agent *a) {
	fprintf(stream, "\nagent:      {\n");
	fprintf(stream, "\tindex:      %zu\n", a->index);
	fprintf(stream, "\tpos:        [%d;%d]\n", a->pos.x, a->pos.y);
	fprintf(stream, "\tc_state:    %d\n", a->current_state);
	fprintf(stream, "\tdirection:  %s\n", direction_as_cstr(a->direction));
	fprintf(stream, "\thunger:     %d\n", a->hunger);
	fprintf(stream, "\thealth:     %d\n", a->health);
	fprintf(stream, "\tlifetime:   %zu\n", a->lifetime);
	fprintf(stream, "\thistory:    {\n");
	for (size_t i = 0; i < a->lifetime; ++i)
		fprintf(stream, "\t\t%3zu:    %s\n", i, action_as_cstr(a->history[i]));
	fprintf(stream, "\t}\n");
	fprintf(stream, "\tchromosomes:    {\n");
	print_chromosome(stream, &a->chromosome, a->index);
	fprintf(stream, "\t}\n");
	fprintf(stream, "}\n");
}

void print_the_state_of_oldest_agent(Game *game) {
	Agent *oldest_agent = &game->agents[0];
	for (size_t i = 1; i < AGENTS_COUNT; ++i) {
		if (game->agents[i].lifetime > oldest_agent->lifetime)
			oldest_agent = &game->agents[i];
	}
	print_agent_verbose(stdout, oldest_agent);
}

Agent *get_ptr_to_agent_at_pos(Game *game, Position pos) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i)
		if (positions_are_equal(game->agents[i].pos, pos))
			return &game->agents[i];

	return NULL;
}

void initialize_game(Game *game) {
	memset(game, 0, sizeof(*game));

	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		initialize_basic_agent_properties(game, &game->agents[i], i);

		for (size_t j = 0; j < GENES_COUNT; ++j) {
			initialize_gene(&game->agents[i].chromosome.genes[j]);
		}
	}

	initialize_food(game);
	initialize_walls(game);
}

void game_step(Game *game) {
	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		Agent *agent = &game->agents[i];

		agent->lifetime += 1;

		if (agent->lifetime == MAX_LIFETIME) {
			agent->health = 0;
			continue;
		}

		for (size_t j = 0; j < GENES_COUNT; ++j) {
			Gene *gene = &game->agents[i].chromosome.genes[j];

			if (gene->current_state != agent->current_state)
				continue;

			if (gene->environment != interpret_environment_infront_of_agent(game, agent))
				continue;

			if (agent->health <= 0)
				continue;

			execute_action(game, agent, gene->action);
			agent->current_state = gene->next_state;
			break;
		}
	}

	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		if (game->agents[i].hunger >= LETHAL_HUNGER) {
			game->agents[i].hunger = LETHAL_HUNGER;
			game->agents[i].health -= HUNGER_TICK;
			continue;
		}

		game->agents[i].hunger += HUNGER_TICK;
	}
}

const char *env_as_cstr(Environment env) {
	switch (env) {
	case ENV_NOTHING: return "ENV_NOTHING";
	case ENV_AGENT: return "ENV_AGENT";
	case ENV_FOOD: return "ENV_FOOD";
	case ENV_WALL: return "ENV_WALL";
	case ENV_COUNT:
	default: assert(0 && "That's not supposed to happen."); return NULL;
	}
}

const char *action_as_cstr(AgentAction a) {
	switch (a) {
	case AA_NOTHING: return "AA_NOTHING";
	case AA_STEP: return "AA_STEP";
	case AA_TURN_LEFT: return "AA_TURN_LEFT";
	case AA_TURN_RIGHT: return "AA_TURN_RIGHT";
	case AA_COUNT:
	default: assert(0 && "That's not supposed to happen."); return NULL;
	}
}

const char *direction_as_cstr(Direction d) {
	switch (d) {
	case DIR_RIGHT: return "DIR_RIGHT";
	case DIR_UP: return "DIR_UP";
	case DIR_LEFT: return "DIR_LEFT";
	case DIR_DOWN: return "DIR_DOWN";
	}
}

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
	return (Environment)random_int_range(0, ENV_COUNT);
}

AgentAction random_action(void) {
	return (AgentAction)random_int_range(AA_NOTHING + 1, AA_COUNT);
}

void initialize_gene(Gene *gene) {
	gene->current_state = random_int_range(0, GENES_COUNT);
	gene->environment = random_environment();
	gene->action = random_action();
	gene->next_state = random_int_range(0, GENES_COUNT);
}

void initialize_basic_agent_properties(Game *game, Agent *agent, size_t agent_index) {
	agent->index = agent_index;
	agent->pos = random_empty_position(game);
	agent->direction = random_direction();
	agent->hunger = STARTING_HUNGER;
	agent->health = STARTING_HEALTH;
	agent->lifetime = 0;
	agent->history[0] = AA_NOTHING;

	// qm_todo: improve this later.
	agent->direction = agent_index % 4;
}

void initialize_food(Game *game) {
	for (size_t i = 0; i < FOOD_COUNT; ++i) {
		game->food[i].quantity = random_int_range(0, FOOD_QUANTITY_GENERATION_MAX);
		game->food[i].pos = random_empty_position(game);
	}
}

void initialize_walls(Game *game) {
	for (size_t i = 0; i < WALLS_COUNT; ++i) {
		game->walls[i].pos = random_empty_position(game);
	}
}

int mod_int(int first, int second) {
	return (first % second + second) % second;
}

void move_agent(Agent *agent) {
	Position delta = position_directions[agent->direction];

	agent->pos.x = mod_int(agent->pos.x + delta.x, BOARD_WIDTH);
	agent->pos.y = mod_int(agent->pos.y + delta.y, BOARD_HEIGHT);
}

Position get_position_infront_of_agent(const Agent *agent) {
	Position delta = position_directions[agent->direction];
	Position next = agent->pos;

	next.x += mod_int(next.x + delta.x, BOARD_WIDTH);
	next.y += mod_int(next.y + delta.y, BOARD_HEIGHT);

	return next;
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

Environment interpret_environment_infront_of_agent(Game *game, Agent *agent) {
	// This order kind of serves as priority list.

	if (get_ptr_to_food_infront_of_agent(game, agent) != NULL)
		return ENV_FOOD;

	if (get_ptr_to_agent_infront_of_agent(game, agent) != NULL)
		return ENV_AGENT;

	if (get_ptr_to_wall_infront_of_agent(game, agent) != NULL)
		return ENV_WALL;

	return ENV_NOTHING;
}

void execute_action(Game *game, Agent *agent, AgentAction action) {
	agent->history[agent->lifetime] = action;

	// printf("Agent %zu performs action: %s\n", agent->index, action_as_cstr(action));

	switch (action) {
	case AA_NOTHING: break;

	case AA_STEP: {
		Food *food = get_ptr_to_food_infront_of_agent(game, agent);
		Agent *victim = get_ptr_to_agent_infront_of_agent(game, agent);
		Wall *wall = get_ptr_to_wall_infront_of_agent(game, agent);

		if (food != NULL) {
			printf("\t\tAgent %zu ate the food!\n", agent->index);
			food->quantity -= 1;
			agent->hunger -= FOOD_HUNGER_RECOVERY;

			if (agent->hunger < 0)
				agent->hunger = 0;
		} else if (victim != NULL) {
			printf("\t\tAgent %zu performed an attack!\n", agent->index);
			victim->health -= ATTACK_DMG;
			agent->health -= RETALIATION_DMG;

			// No check for negative hp here.
			// We perform all actions first, then declare dead agents.
		} else if (wall == NULL) {
			printf("\t\tAgent %zu just steped forward and that's it.\n", agent->index);
			move_agent(agent);
		}
	} break;

	case AA_TURN_LEFT:
		// this is absolutely brilliant!
		agent->direction = (Direction)mod_int((int)agent->direction + 1, 4);
		break;

	case AA_TURN_RIGHT: agent->direction = (Direction)mod_int((int)agent->direction - 1, 4); break;

	case AA_COUNT:
	default: assert(0 && "This is not supposed to happen, fix the 'action' value."); break;
	}
}

// qm_todo: different mating strategies? second chances?
void mate_agents(const Agent *parent_a, const Agent *parent_b, Agent *child) {
	const size_t OFFSET = GENES_COUNT / 2;
	const size_t GENE_SIZE = sizeof(Gene);

	memcpy(child->chromosome.genes, parent_a->chromosome.genes, OFFSET * GENE_SIZE);
	memcpy(child->chromosome.genes + OFFSET, parent_b->chromosome.genes + OFFSET, OFFSET * GENE_SIZE);
}

void mutate_agent(Agent *agent) {
	// very crude mutation algorithm, but it works
	// qm_todo: improve it later.
	for (size_t i = 0; i < GENES_COUNT; ++i) {
		if (random_int_range(0, MUTATION_PROBABILITY) < MUTATION_THRESHHOLD) {
			initialize_gene(&agent->chromosome.genes[i]);
		}
	}
}

int agent_lifetime_comparator(const void *a, const void *b) {
	return (int)(((const Agent *)b)->lifetime - ((const Agent *)a)->lifetime);
}

// This function is genious!
//
// It sorts agents in descending order based on their lifetime.
//
// Best of them (in index range [0; MATING_SELECTION_POOL)) will be used to create
// chromosomes for the next game.
//
// On top of having the two halfs of the best genotypes, a new agent has a chance to undergo a
// mutation which can change some of his genes (for better of worse).
//
// Everything else is just a basic setup of game properties.
void prepare_next_game(Game *previous_game, Game *next_game) {
	memset(next_game, 0, sizeof(*next_game));

	qsort(previous_game->agents, AGENTS_COUNT, sizeof(Agent), agent_lifetime_comparator);

	// qm_todo: should I regenerate it or copy from previous game?
	// initialize_food(next_game);
	// initialize_walls(next_game);
	memcpy(next_game->food, previous_game->food, FOOD_COUNT * sizeof(Food));
	memcpy(next_game->walls, previous_game->walls, WALLS_COUNT * sizeof(Wall));

	for (size_t i = 0; i < AGENTS_COUNT; ++i) {
		size_t parent_a_index = (size_t)random_int_range(0, MATING_SELECTION_POOL);
		size_t parent_b_index = (size_t)random_int_range(0, MATING_SELECTION_POOL);

		mate_agents(&previous_game->agents[parent_a_index],
			    &previous_game->agents[parent_b_index],
			    &next_game->agents[i]);

		mutate_agent(&next_game->agents[i]);
		initialize_basic_agent_properties(next_game, &next_game->agents[i], i);
	}
}

void dump_game_state(const char* filepath, const Game *game) {
	FILE *state_dump_file_handle = fopen(filepath, "wb");

	if (state_dump_file_handle == NULL) {
		fprintf(stderr, "ERROR: Couldn't open the file to dump the game's state.\n");
                return;
	}

	fwrite(game, sizeof(*game), 1, state_dump_file_handle);
	if (ferror(state_dump_file_handle)) {
		fprintf(stderr, "ERROR: Couldn't write the file to dump the game's state.\n");
	} else {
		fprintf(stdout, "INFO: Game state was successfully written into a file.\n");
	}

	fclose(state_dump_file_handle);
}

void load_game_state(const char* filepath, Game *game) {
	FILE *state_dump_file_handle = fopen(filepath, "rb");

	if (state_dump_file_handle == NULL) {
		fprintf(stderr, "ERROR: Couldn't open the file to dump the game's state.\n");
                return;
	}

	size_t read_chunks_count = fread(game, sizeof(*game), 1, state_dump_file_handle);
	if (ferror(state_dump_file_handle) || read_chunks_count != 1) {
		fprintf(stderr, "ERROR: Couldn't write the file to dump the game's state.\n");
	} else {
		fprintf(stdout, "INFO: Game state was successfully read from a file.\n");
	}

	fclose(state_dump_file_handle);
}

bool is_everyone_dead(const Game *game) {
        for (size_t i = 0; i < AGENTS_COUNT; ++i) {
                if (game->agents[i].health > 0)
                        return false;
        }
        return true;
}

