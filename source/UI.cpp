#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
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
	bool ItemSelected = false;
};

bool CheckButtonPressed(SDL_Rect* ButtonRect, int TouchX, int TouchY)
{
	if(TouchX > ButtonRect->x && TouchX < ButtonRect->x + ButtonRect->w && TouchY > ButtonRect->y && TouchY < ButtonRect->y + ButtonRect->h) return true;
	else return false;
}

void ScrollList::DrawList()
{
	ItemSelected = false;
	int ListingHeight = ListHeight / ListingsOnScreen;
	int ListLength = ListingsOnScreen;
	if(ListLength > ListingTextVec.size()) ListLength = ListingTextVec.size();
	
	//Check cursor is with in bounds
	if(CursorIndex > ListLength-1)
	{
		CursorIndex = ListLength-1;
		ListRenderOffset++;
	}
	if(CursorIndex < 0)
	{
		CursorIndex = 0;
		ListRenderOffset--;
	}
	
	//Check selected index is with in the bounds
	if(SelectedIndex < 0)
	{
		SelectedIndex = ListingTextVec.size()-1;
		CursorIndex = ListLength;
		ListRenderOffset = ListingTextVec.size() - ListLength-1;
		//Don't crash if we scroll up and don't have enough items in the list to show
		if(ListRenderOffset < 0)
		{
			SelectedIndex = ListingTextVec.size()-1;
			CursorIndex = ListingTextVec.size()-1;
			ListRenderOffset = 0;
		}
	}
	else if(SelectedIndex > ListingTextVec.size()-1)
	{
		SelectedIndex = 0;
		CursorIndex = 0;
		ListRenderOffset = 0;
	}
	
	//Draw the Amiibo list
	for(int i = 0; i < ListLength; i++)
	{
		//Set the background color with alternating colours
		if(i % 2 == 1)
		{
			SDL_SetRenderDrawColor(renderer, 0, 184, 212, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(renderer, 0, 229, 255, 255);
		}
		//Check if this is the highlighted file
		if(i == CursorIndex)
		{
			SDL_SetRenderDrawColor(renderer, 224, 247, 250, 255);
		}
		
		SDL_Rect MenuItem = {0, ListYOffset + (i * ListingHeight), ListWidth, ListingHeight};
		SDL_RenderFillRect(renderer, &MenuItem);
		
		//Draw file names
		SDL_Color TextColour = {0, 0, 0};
		SDL_Surface* FileNameSurface = TTF_RenderUTF8_Blended_Wrapped(ListFont, ListingTextVec.at(i + ListRenderOffset).c_str(), TextColour, ListWidth);
		SDL_Texture* FileNameTexture = SDL_CreateTextureFromSurface(renderer, FileNameSurface);
		SDL_Rect AmiiboNameRect = {0, MenuItem.y + ((MenuItem.h - FileNameSurface->h) / 2), FileNameSurface->w, FileNameSurface->h};
		SDL_RenderCopy(renderer, FileNameTexture, NULL, &AmiiboNameRect);
		
		//Check if option is pressed
		if(CheckButtonPressed(&MenuItem, *TouchListX, *TouchListY))
		{
			int SelectedDifference = i - CursorIndex;
			SelectedIndex += SelectedDifference;
			CursorIndex = i;
			ItemSelected = true;
		}
	
		//Clean up
		SDL_DestroyTexture(FileNameTexture);
		SDL_FreeSurface(FileNameSurface);
	}
}