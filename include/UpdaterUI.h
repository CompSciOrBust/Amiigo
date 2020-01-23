//A lot of code for this was modified from Goldleaf. Thank you Xor (Again).
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
//#include <UI.h>
#include "Networking.h"
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
	std::string LatestID;
	public:
	UpdaterUI();
	void DrawUI();
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
	bool NewVersion;
	std::string NROPath = "sdmc:/switch/Amiigo.nro";
};