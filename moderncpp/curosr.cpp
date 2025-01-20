#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

float easeOutQuad(float t) { return t * (2 - t); }

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("Cursor Jump Effect", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
										  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr)
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr)
	{
		SDL_DestroyWindow(window);
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// Main loop
	bool running = true;
	SDL_Event event;
	int cursorX = WINDOW_WIDTH / 2;
	int cursorY = WINDOW_HEIGHT / 2;
	int targetX = cursorX;
	int targetY = cursorY;
	float speed = 0.1f; // Speed of the cursor animation
	float t = 0.0f; // Time variable for easing

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_LEFT:
					targetX -= 50;
					t = 0.0f; // Reset time for new animation
					break;
				case SDLK_RIGHT:
					targetX += 50;
					t = 0.0f;
					break;
				case SDLK_UP:
					targetY -= 50;
					t = 0.0f;
					break;
				case SDLK_DOWN:
					targetY += 50;
					t = 0.0f;
					break;
				}
			}
		}

		// Animate cursor with easing
		if (t < 1.0f)
		{
			t += speed;
			float easedT = easeOutQuad(t);
			cursorX += (targetX - cursorX) * easedT;
			cursorY += (targetY - cursorY) * easedT;
		}

		// Clear screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Draw cursor
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_Rect cursorRect = {cursorX - 5, cursorY - 5, 10, 10};
		SDL_RenderFillRect(renderer, &cursorRect);

		// Present renderer
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
