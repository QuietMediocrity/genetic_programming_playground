#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"

#define TRAINING_THRESHHOLD 2048 

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	srand((unsigned int)time(0));

	const char *filepath = "./output/game_state.bin";
	Game games[2] = { 0 };
	int current_game = 0;
	load_game_state(filepath, &games[current_game]);

	for (size_t i = 0; i < TRAINING_THRESHHOLD; ++i) {
		fprintf(stdout, "Generation `%zu`.\n", i + 1);

		while (!is_everyone_dead(&games[current_game]))
			game_step(&games[current_game]);

		print_the_state_of_oldest_agent(&games[current_game]);

		int next = 1 - current_game;
		prepare_next_game(&games[current_game], &games[next]);
		current_game = next;
	}

	dump_game_state(filepath, &games[current_game]);
	return 0;
}
