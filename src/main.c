#include "./game.h"
#include "./rendering.h"

#include "SDL_events.h"
#include <stddef.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	printf("hello\n");
	srand((unsigned int)time(0));

	Game game;
	initialize_game(&game);

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
					initialize_game(&game);
				} break;
				case SDLK_s: { // qm_todo: later on, make the ticks automatic after delta t, not manual.
					game_step(&game);
				} break;
				}
			} break;
			case SDL_MOUSEBUTTONDOWN: {
				Position click_pos = {
					(int)floorf((float)event.button.x / CELL_WIDTH),
					(int)floorf((float)event.button.y / CELL_HEIGHT),
				};

				Agent *agent_at_pos = get_ptr_to_agent_at_pos(&game, click_pos);

				if (agent_at_pos == NULL) {
					printf("Click on the agent, dumbass.");
					break;
				}

				print_agent(stdout, agent_at_pos);
			} break;
			}
		}

		clear_board(renderer);
		render_game(renderer, &game);

		SDL_RenderPresent(renderer);
	}

	Agent *oldest_agent = &game.agents[0];
	for (size_t i = 1; i < AGENTS_COUNT; ++i) {
		if (game.agents[i].lifetime > oldest_agent->lifetime)
			oldest_agent = &game.agents[i];
	}
	print_agent_verbose(stdout, oldest_agent);

	SDL_Quit();
	return 0;
}
