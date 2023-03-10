#include "./game.h"
#include "./rendering.h"

#include "SDL_events.h"
#include <stddef.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	srand((unsigned int)time(0));

	Game games[2] = { 0 };
	int current_game = 0;
	initialize_game(&games[current_game]);

	scc(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window *window =
		SDL_CreateWindow("QM's playground", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
	scp(window);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	scp(renderer);

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
				case SDLK_q: {
					quit = 1;
				} break;
				case SDLK_r: {
					initialize_game(&games[current_game]);
				} break;
				case SDLK_s: { // qm_todo: later on, make the ticks automatic after delta t, not manual.
					game_step(&games[current_game]);
				} break;
				case SDLK_d: {
					dump_game_state("./output/game_state.bin", &games[current_game]);
				} break;
				case SDLK_l: {
					load_game_state("./output/game_state.bin", &games[current_game]);
				} break;
				case SDLK_n: {
					int next = 1 - current_game;
					print_the_state_of_oldest_agent(&games[current_game]);
					prepare_next_game(&games[current_game], &games[next]);
					current_game = next;
				} break;
				}
			} break;
			case SDL_MOUSEBUTTONDOWN: {
				Position click_pos = {
					(int)floorf((float)event.button.x / CELL_WIDTH),
					(int)floorf((float)event.button.y / CELL_HEIGHT),
				};

				Agent *agent_at_pos = get_ptr_to_agent_at_pos(&games[current_game], click_pos);
				Food *food_at_pos = get_ptr_to_food_at_pos(&games[current_game], click_pos);
				Wall *wall_at_pos = get_ptr_to_wall_at_pos(&games[current_game], click_pos);

				if (agent_at_pos != NULL) {
					print_agent_verbose(stdout, agent_at_pos);
				}
				if (food_at_pos != NULL) {
					fprintf(stdout,
						"Food at [%d;%d] with quantity: %d\n",
						food_at_pos->pos.x,
						food_at_pos->pos.y,
						food_at_pos->quantity);
				}
				if (wall_at_pos != NULL) {
					fprintf(stdout, "Wall at [%d;%d]\n", wall_at_pos->pos.x, wall_at_pos->pos.y);
				}

				fflush(stdout);
			} break;
			}
		}

		clear_board(renderer);
		render_game(renderer, &games[current_game]);

		SDL_RenderPresent(renderer);
	}

	print_the_state_of_oldest_agent(&games[current_game]);

	SDL_Quit();
	return 0;
}
