//A lot of code for this was modified from Goldleaf. Thank you Xor (Again).
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <UI.h>
#include "networking.h"
#include "nlohmann/json.hpp"
#include <fstream>
using namespace std;
using json = nlohmann::json;

class UpdaterUI
{
	private:
	void DrawText(std::string);
	int UpdateState = 0;
	bool CheckForNewVersion();
	std::string UpdateText = " ";
	TTF_Font *TextFont;
	SDL_Color TextColour = {0, 0, 0};
	json GitAPIData;
	string LatestID;
	public:
	UpdaterUI();
	void DrawUI();
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
};

UpdaterUI::UpdaterUI()
{
	nifmInitialize(); //Init nifm for connection stuff
	TextFont = GetSharedFont(48);
}

void UpdaterUI::DrawUI()
{
	//Handle input
	bool BPressed = false;
	while (SDL_PollEvent(Event))
	{
		switch (Event->type)
		{
			case SDL_JOYBUTTONDOWN:
			if (Event->jbutton.which == 0)
			{
				//Plus pressed
				if (Event->jbutton.button == 10)
				{
					*IsDone = 1;
				}
				//B pressed
				else if(Event->jbutton.button == 1)
				{
					BPressed = true;
				}
			}
		}
		break;
	}
	
	//Do the update stages
	switch(UpdateState)
	{
		//Check connection stage
		case 0:
		{
			//Check we have a connection before trying to access the network
			if(HasConnection())
			{
				UpdateState++;
			}
			else
			{
				UpdateText = "Waiting for connection.";
			}
		}
		break;
		//Check if newer version stage
		case 1:
		{
			if(CheckForNewVersion())
			{
				UpdateState++;
				UpdateText = "Downloading " + LatestID + ". This might take a while.";
			}
			else
			{
				UpdateText = "Already on the latest version.";
				if(BPressed)
				{
					*WindowState = 0;
				}
			}
		}
		break;
		//Download update stage
		case 2:
		{
			string UpdateFileURL = "https://github.com/CompSciOrBust/Amiigo/releases/download/" + LatestID + "/Amiigo.nro";
			RetrieveToFile(UpdateFileURL, "sdmc:/switch/Failed_Amiigo_Update.nro");
			remove("sdmc:/switch/Amiigo.nro");
			rename("sdmc:/switch/Failed_Amiigo_Update.nro", "sdmc:/switch/Amiigo.nro");
			*IsDone = 1;
		}
		break;
	}
	DrawText(UpdateText);
}

bool UpdaterUI::CheckForNewVersion()
{
	//Get data from GitHub API
	string Data = RetrieveContent("https://api.github.com/repos/CompSciOrBust/Amiigo/releases", "application/json");
	GitAPIData = json::parse(Data);
    LatestID = GitAPIData[0]["tag_name"].get<std::string>();
	//Check if we're running the latest version
	return (LatestID != APP_VERSION);
}

void UpdaterUI::DrawText(std::string Message)
{
	//Draw the rect
	SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
	SDL_Rect MessageRect = {0,0, *Width, *Height};
	SDL_RenderFillRect(renderer, &MessageRect);
	//Draw the text
	SDL_Surface* MessageTextSurface = TTF_RenderUTF8_Blended_Wrapped(TextFont, Message.c_str(), TextColour, *Width);
	SDL_Texture* MessagerTextTexture = SDL_CreateTextureFromSurface(renderer, MessageTextSurface);
	SDL_Rect HeaderTextRect = {(*Width - MessageTextSurface->w) / 2, (*Height - MessageTextSurface->h) / 2, MessageTextSurface->w, MessageTextSurface->h};
	SDL_RenderCopy(renderer, MessagerTextTexture, NULL, &HeaderTextRect);
	//Clean up
	SDL_DestroyTexture(MessagerTextTexture);
	SDL_FreeSurface(MessageTextSurface);
}