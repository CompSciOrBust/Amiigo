#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <switch.h>
#include "Utils.h"
using namespace std;

//Global vars
int BorderSize = 3;

//Colors taken from here
//https://material-ui.com/customization/color/

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
		//Set the background color
		if(IsActive) DrawJsonColorConfig(renderer, "UI_background");
		else DrawJsonColorConfig(renderer, "UI_background_alt");
		//Check if this is the highlighted file
		if(i == CursorIndex && IsActive)
		{
			DrawJsonColorConfig(renderer, "UI_cursor");
			//Cyan 50
			//if(IsActive) SDL_SetRenderDrawColor(renderer, 224, 247, 250, 255);
			//else SDL_SetRenderDrawColor(renderer, 232, 234, 246, 255); //Indigo
		}
		
		SDL_Rect MenuItem = {ListXOffset, ListYOffset + (i * ListingHeight), ListWidth, ListingHeight};
		SDL_RenderFillRect(renderer, &MenuItem);
		
		//Draw file names
		SDL_Color TextColour = {0, 0, 0};
		SDL_Surface* FileNameSurface = TTF_RenderUTF8_Blended_Wrapped(ListFont, ListingTextVec.at(i + ListRenderOffset).c_str(), TextColour, ListWidth);
		SDL_Texture* FileNameTexture = SDL_CreateTextureFromSurface(renderer, FileNameSurface);
		//Calculate text X and Y Coords
		int TextY = MenuItem.y + ((MenuItem.h - FileNameSurface->h) / 2);
		int TextX = 0;
		if(CenterText)
		{
			TextX = MenuItem.x + ((MenuItem.w - FileNameSurface->w) / 2);
		}
		else
		{
			TextX = MenuItem.x;
		}
		SDL_Rect AmiiboNameRect = {TextX, TextY, FileNameSurface->w, FileNameSurface->h};
		SDL_RenderCopy(renderer, FileNameTexture, NULL, &AmiiboNameRect);
		
		//Draw borders
		DrawJsonColorConfig(renderer, "UI_borders_list");
		SDL_Rect BorderRect = {ListXOffset, ListYOffset + (i * ListingHeight) - 1, ListWidth, BorderSize};
		SDL_RenderFillRect(renderer, &BorderRect);
		//Check if we need to draw one more border
		if(ListLength < ListingsOnScreen && i == ListLength - 1)
		{
			BorderRect = {ListXOffset, ListYOffset + (++i * ListingHeight) - 1, ListWidth, BorderSize};
			SDL_RenderFillRect(renderer, &BorderRect);
		}
		
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

//Thank you to Nichole Mattera for telling me how to do this
TTF_Font *GetSharedFont(int FontSize)
{
	PlFontData standardFontData;
	plGetSharedFontByType(&standardFontData, PlSharedFontType_Standard);
	return TTF_OpenFontRW(SDL_RWFromMem(standardFontData.address, standardFontData.size), 1, FontSize);
}

void DrawButtonBorders(SDL_Renderer* renderer, ScrollList *LeftList, ScrollList *MenuList, int HeaderHeight, int FooterHeight, int Width, int Height, bool SplitFooter)
{
	DrawJsonColorConfig(renderer, "UI_borders");
	//Draw border for the two lists
	SDL_Rect BorderRect = {MenuList->ListXOffset, MenuList->ListYOffset, BorderSize, MenuList->ListHeight};
	SDL_RenderFillRect(renderer, &BorderRect);
	//Draw border for the header
	BorderRect = {0, HeaderHeight, Width, BorderSize};
	SDL_RenderFillRect(renderer, &BorderRect);
	//Draw the footer border
	BorderRect = {0, Height - FooterHeight, Width, BorderSize};
	SDL_RenderFillRect(renderer, &BorderRect);
	if(!SplitFooter) return;
	//Draw the footer button border
	BorderRect = {Width/2, Height - FooterHeight, BorderSize, Height};
	SDL_RenderFillRect(renderer, &BorderRect);
}