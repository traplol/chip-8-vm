/*
	Copyright(c) 2014 Max Mickey

	This software is provided 'as-is', without any express or implied
	warranty.In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions :

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software.If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution. 
*/

#include "chip8.h"
#include "SDL.h"

Chip8 emu;
SDL_Window* window;
SDL_Renderer* renderer;

int quit;

bool setup_graphics();
void setup_input();
void draw_graphics();
void draw_pixel(int x, int y);
int handle_inputs();
void set_input_state(SDL_Keycode sym, int state);

int main(int argc, char *argv[])
{
	
	if (argc != 2)
	{
		printf("useage: %s <rom file>", argv[0]);
		return 0;
	}
	if (setup_graphics())
	{
		setup_input();
		emu.init();
		if (emu.load_rom(argv[1]))
		{
			while (!quit)
			{
				static double start = std::clock();
				double time_passed = std::clock();
				if ((time_passed - start) >= EMU_TICK_RATE)
				{
					start = std::clock();
#ifdef DEBUGGER
					if (handle_inputs())
					{
						emu.run_cycle();
					}
#else
					handle_inputs();
					emu.run_cycle();
#endif
					if (emu.draw_flag)
					{
						draw_graphics();
						emu.draw_flag = false;
					}
				}
			}
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
		}
	}
	return 0;
}

bool setup_graphics()
{
	bool success = true;
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Chip 8 emu",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		GFX_WIDTH * 10,
		GFX_HEIGHT * 10,
		SDL_WINDOW_OPENGL);
	if (window == NULL)
	{
		printf("couldn't create window, aborting!\n");
		success = false;
	}
	if (success)
	{
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == NULL)
		{
			printf("couldn't create renderer, aborting!\n");
			success = false;
		}
	}
	return success;
}

void setup_input()
{

}

void draw_graphics()
{
	SDL_RenderClear(renderer);

	for (int y = 0; y < GFX_HEIGHT; ++y)
	{
		for (int x = 0; x < GFX_WIDTH; ++x)
		{
			if (emu.GFX[(y * 64) + x] == 0)
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
			}
			draw_pixel(x,y);
		}
		
	}
	SDL_RenderPresent(renderer);
}

void draw_pixel(int x, int y)
{
	SDL_Rect rect{ x * 10, y * 10, 10, 10 };
	SDL_RenderFillRect(renderer, &rect);
}

int handle_inputs()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			quit = 1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			break;
		case SDL_KEYDOWN:
#ifdef DEBUGGER
			// simulating debugger stepping.
			if (e.key.keysym.sym == SDLK_RETURN)
			{
				return 1;
			}
#endif
			set_input_state(e.key.keysym.sym, 1);
			break;
		case SDL_KEYUP:
			set_input_state(e.key.keysym.sym, 0);
			break;
		default: 
			break;
		}
	}
	return 0;
}
void set_input_state(SDL_Keycode sym, int state)
{
	switch (sym)
	{
	case SDLK_1:
		emu.KEY_STATE[1] = state;
		break;
	case SDLK_2:
		emu.KEY_STATE[2] = state;
		break;
	case SDLK_3:
		emu.KEY_STATE[3] = state;
		break;
	case SDLK_4:
		emu.KEY_STATE[0xC] = state;
		break;
	case SDLK_q:
		emu.KEY_STATE[4] = state;
		break;
	case SDLK_w:
		emu.KEY_STATE[5] = state;
		break;
	case SDLK_e:
		emu.KEY_STATE[6] = state;
		break;
	case SDLK_r:
		emu.KEY_STATE[0xD] = state;
		break;
	case SDLK_a:
		emu.KEY_STATE[7] = state;
		break;
	case SDLK_s:
		emu.KEY_STATE[8] = state;
		break;
	case SDLK_d:
		emu.KEY_STATE[9] = state;
		break;
	case SDLK_f:
		emu.KEY_STATE[0xE] = state;
		break;
	case SDLK_z:
		emu.KEY_STATE[0xA] = state;
		break;
	case SDLK_x:
		emu.KEY_STATE[0] = state;
		break;
	case SDLK_c:
		emu.KEY_STATE[0xB] = state;
		break;
	case SDLK_v:
		emu.KEY_STATE[0xF] = state;
		break;
	default:
		break;
	}
}