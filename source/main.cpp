#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <switch.h>
#include <AmiigoUI.h>
#include <nfpemu.h>
int main(int argc, char *argv[])
{
	//Vars
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    int done = 0;
	int Width = 1280;
	int Height = 720;
	int WindowState = 0;
	
    // mandatory at least on switch, else gfx is not properly closed
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    // create an SDL window (OpenGL ES2 always enabled)
    // when SDL_FULLSCREEN flag is not set, viewport is automatically handled by SDL (use SDL_SetWindowSize to "change resolution")
    // available switch SDL2 video modes :
    // 1920 x 1080 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
    // 1280 x 720 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
    window = SDL_CreateWindow("sdl2_gles2", 0, 0, Width, Height, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // create a renderer (OpenGL ES2)
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // open CONTROLLER_PLAYER_1 and CONTROLLER_PLAYER_2
    // when railed, both joycons are mapped to joystick #0,
    // else joycons are individually mapped to joystick #0, joystick #1, ...
    // https://github.com/devkitPro/SDL/blob/switch-sdl2/src/joystick/switch/SDL_sysjoystick.c#L45
    for (int i = 0; i < 2; i++) {
        if (SDL_JoystickOpen(i) == NULL) {
            SDL_Log("SDL_JoystickOpen: %s\n", SDL_GetError());
            SDL_Quit();
            return -1;
        }
    }

	romfsInit(); //Init romfs to load font
	TTF_Init(); //Init the font
	nfpemuInitialize(); //Init nfp ipc

	//Give UI access to vars
	AmiigoUI *MainUI = new AmiigoUI();
	MainUI->Event = &event;
	MainUI->WindowState = &WindowState;
	MainUI->renderer = renderer;
	MainUI->Width = &Width;
	MainUI->Height = &Height;
	MainUI->IsDone = &done;
    while (!done)
	{
		//Clear the frame
        SDL_RenderClear(renderer);

		switch(WindowState)
		{
			//Draw the main UI
			case 0:
			{
				MainUI->DrawUI();
			}
			break;
		}

		//Draw the frame
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
