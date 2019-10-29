#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <switch.h>
using namespace std;

class ScrollList
{
	public:
	void DrawList();
	int ListHeight = 0;
	int ListWidth = 0;
	int SelectedIndex = 0;
	int CursorIndex = 0;
	int ListRenderOffset = 0;
	int ListingsOnScreen = 0;
	int *TouchListX = NULL;
	int *TouchListY = NULL;
	vector <string> ListingTextVec = vector <string>(0);
	SDL_Renderer *renderer;
	TTF_Font *ListFont;
	int ListYOffset = 0;
	int ListXOffset = 0;
	bool ItemSelected = false;
	bool IsActive = false;
	bool CenterText = false;
};

bool CheckButtonPressed(SDL_Rect*, int, int);
TTF_Font *GetSharedFont(int FontSize);
void DrawButtonBorders(SDL_Renderer*, ScrollList*, ScrollList*, int, int, int, int, bool);