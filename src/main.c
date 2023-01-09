#include "./game.h"
#include "./rendering.h"

#include <stddef.h>
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

                clear_board(renderer);
		render_board_grid(renderer);
		render_game(renderer, &game);

		SDL_RenderPresent(renderer);
	}

	SDL_Quit();

	return 0;
}
