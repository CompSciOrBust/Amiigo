#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "nlohmann/json.hpp"
#include "Networking.h"
#include <fstream>
#include <dirent.h>
#include <vector>
//#include <UI.h>
using namespace std;
using json = nlohmann::json;

class AmiiboVars
{
	public:
	string AmiiboSeries = "";
	string AmiiboName = "";
	int ListIndex = 0;
};

class CreatorUI
{
	private:
	TTF_Font *HeaderFont;
	TTF_Font *ListFont;
	SDL_Color TextColour = {0, 0, 0};
	void DrawHeader();
	void DrawAmiiboList();
	void DrawSeriesList();
	void DrawFooter();
	int HeaderHeight;
	int FooterHeight;
	int ListHeight;
	int TouchX = -1;
	int TouchY = -1;
	json JData;
	int JDataSize = 0;
	bool HasSelectedSeries = false;
	vector<string> SeriesVec;
	vector<AmiiboVars> AmiiboVarsVec;
	vector<AmiiboVars> SortedAmiiboVarsVec;
	string AmiiboAPIString = "";
	void PleaseWait(string mensage);
	public:
	CreatorUI();
	void GetInput();
	void DrawUI();
	void GetDataFromAPI(string);
	void InitList();
	void ListSelect();
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
	ScrollList *SeriesList;
	ScrollList *MenuList;
	int SeriesListWidth;
	string *CurrentPath;
};