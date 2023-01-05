#include <SDL2/SDL_render.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

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

int main(int argc, char *argv[])
{
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
			}
		}

		scc(SDL_SetRenderDrawColor(renderer, 36, 41, 46, 255));
		scc(SDL_RenderClear(renderer));
                SDL_RenderPresent(renderer);
	}

	SDL_Quit();
	return 0;
}
