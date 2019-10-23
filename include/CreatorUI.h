//We could probably derive this from a common UI class along with AmiigoUI but I didn't plan for that and I'm tired
//Dear Nintendo. Don't C&D me please. Go after these guys instead amiiboapi.com.
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "nlohmann/json.hpp"
#include "networking.h"
#include <fstream>
#include <dirent.h>
#include <vector>
using namespace std;
using json = nlohmann::json;

class CreatorUI
{
	private:
	TTF_Font *HeaderFont;
	TTF_Font *ListFont;
	SDL_Color TextColour = {0, 0, 0};
	void DrawHeader();
	void DrawAmiiboList();
	void DrawSeriesList();
	int HeaderHeight;
	int FooterHeight;
	int ListHeight;
	int SelectedIndex = 0;
	int CursorIndex = 0;
	int ListRenderOffset = 0;
	bool CheckButtonPressed(SDL_Rect*);
	int TouchX = -1;
	int TouchY = -1;
	json JData;
	int JDataSize = 0;
	bool HasSelectedSeries = false;
	vector<string> SeriesVec;
	string AmiiboAPIString = "";
	public:
	CreatorUI();
	void DrawUI();
	void GetDataFromAPI(string);
	string GetKBInput();
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
};