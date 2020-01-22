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
#include <CreatorUI.h>
#include <UpdaterUI.h>
#include <nfpemu.h>
#include <thread>
int main(int argc, char *argv[])
{
std::thread first = std::thread(APIDownloader);
//std::thread second = std::thread(IconDownloader);
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

	TTF_Init(); //Init the font
	plInitialize(); //Init needed for shared font
	if (nfpemuIsAccessible())
	nfpemuInitialize(); //Init nfp ipc
	
	//Give MainUI access to vars
	AmiigoUI *MainUI = new AmiigoUI();
	MainUI->Event = &event;
	MainUI->WindowState = &WindowState;
	MainUI->renderer = renderer;
	MainUI->Width = &Width;
	MainUI->Height = &Height;
	MainUI->IsDone = &done;
	MainUI->InitList();
	
	CreatorUI *AmiigoGenUI = NULL;
	UpdaterUI *UpUI = NULL;
	
	//Make the amiibo folder in case it doesn't exist
	//Not if it exists checking first feels dirty but it doesn't error out. Should we check anyway?
	mkdir("sdmc:/emuiibo/", 0);
	mkdir("sdmc:/emuiibo/amiibo/", 0);
    while (!done)
	{
		//Clear the frame
        SDL_RenderClear(renderer);

		switch(WindowState)
		{
			//Draw the main UI
			case 0:
			{
				MainUI->GetInput();
				MainUI->DrawUI();
				//If the user has switched to the maker UI and the data isn't read show the please wait message
				if(AmiigoGenUI == NULL && WindowState == 1)
				{
					//Display the please wait message
					MainUI->PleaseWait("Please wait while we get data from the Amiibo API...");
				}
			}
			break;
			//Draw the Amiibo creator
			case 1:
			{
				//Check if the UI has been initialized
				if(AmiigoGenUI == NULL)
				{
					//Give AmiigoGenUI access to vars
					AmiigoGenUI = new CreatorUI();
					AmiigoGenUI->Event = &event;
					AmiigoGenUI->WindowState = &WindowState;
					AmiigoGenUI->renderer = renderer;
					AmiigoGenUI->Width = &Width;
					AmiigoGenUI->Height = &Height;
					AmiigoGenUI->IsDone = &done;
					AmiigoGenUI->SeriesListWidth = MainUI->AmiiboListWidth;
					AmiigoGenUI->InitList();
					AmiigoGenUI->MenuList = MainUI->MenuList;
					AmiigoGenUI->CurrentPath = &MainUI->ListDir;
				}
				//Render the UI
				AmiigoGenUI->GetInput();
				AmiigoGenUI->DrawUI();
				//If the window state has changed then we need to rescan the amiibo folder to load the new amiibos in to the list
				if(WindowState == 0)
				{
					MainUI->ScanForAmiibos();
				}
			}
			break;
			//Draw the Amiigo updater
			case 2:
			{
				//Check if the UI has been initialized
				if(UpUI == NULL)
				{
					UpUI = new UpdaterUI();
					UpUI->Event = &event;
					UpUI->WindowState = &WindowState;
					UpUI->renderer = renderer;
					UpUI->Width = &Width;
					UpUI->Height = &Height;
					UpUI->IsDone = &done;
					UpUI->NROPath = argv[0];
				}
				UpUI->DrawUI();
			}
			break;
		}
		
		//If exit option was selected we need to set done to 1
		if(WindowState == 3) done = 1;

		//Draw the frame
        SDL_RenderPresent(renderer);
    }
	
	//join threads before exit
	if (first.joinable())
	{
	socketExit();
	nifmExit();
		MainUI->PleaseWait("Please wait, Thread is Still Working on DataBase...");
		SDL_RenderPresent(renderer);
		first.join();
	}
//	if (second.joinable()) second.join();
	plExit();
	nfpemuExit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
	
    return 0;
}
