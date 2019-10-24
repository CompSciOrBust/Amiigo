#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <UI.h>
using namespace std;

class AmiigoUI
{
	private:
	TTF_Font *HeaderFont;
	TTF_Font *ListFont;
	SDL_Color TextColour = {0, 0, 0};
	void DrawHeader();
	void DrawFooter();
	int HeaderHeight;
	int FooterHeight;
	vector <dirent> Files{vector <dirent>(0)};
	int TouchX = -1;
	int TouchY = -1;
	ScrollList *AmiiboList;
	public:
	AmiigoUI();
	void DrawUI();
	void ScanForAmiibos();
	void PleaseWait();
	void InitList();
	void SetAmiibo(int);
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
};