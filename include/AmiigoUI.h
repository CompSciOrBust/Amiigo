#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <dirent.h>
using namespace std;

class AmiigoUI
{
	private:
	TTF_Font *HeaderFont;
	TTF_Font *ListFont;
	SDL_Color TextColour = {0, 0, 0};
	void DrawHeader();
	void DrawFooter();
	void DrawList();
	int HeaderHeight;
	int FooterHeight;
	int ListHeight;
	int SelectedIndex = 0;
	int CursorIndex = 0;
	int ListRenderOffset = 0;
	vector <dirent> Files{vector <dirent>(0)};
	bool CheckButtonPressed(SDL_Rect*);
	int TouchX = -1;
	int TouchY = -1;
	public:
	AmiigoUI();
	void DrawUI();
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
};