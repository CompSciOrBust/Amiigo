#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <nfpemu.h>
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
	void ScanForAmiibos();
	void PleaseWait();
	SDL_Event *Event;
	int *WindowState;
	SDL_Renderer *renderer;
	int *Width;
	int *Height;
	int *IsDone;
};

AmiigoUI::AmiigoUI()
{
	HeaderFont = TTF_OpenFont("romfs:/font.ttf", 48); //Load the header font
	ListFont = TTF_OpenFont("romfs:/font.ttf", 32); //Load the list font
	//Scan the Amiibo folder for Amiibos
	ScanForAmiibos();
}

void AmiigoUI::DrawUI()
{		

	//This crashes when in the constructor for some reason
	HeaderHeight = (*Height / 100) * 10;
	FooterHeight = (*Height / 100) * 10;
	ListHeight = *Height - HeaderHeight - FooterHeight;
	//Scan input
	while (SDL_PollEvent(Event))
		{
            switch (Event->type)
			{
				//Touchscreen
				case SDL_FINGERDOWN:
				TouchX = Event->tfinger.x * *Width;
				TouchY = Event->tfinger.y * *Height;
				break;
				//TODO: Fix swiping
				/*case SDL_FINGERMOTION:
				//swipe down
				if(Event->tfinger.dy * *Height > 0.25)
				{
					CursorIndex++;
					SelectedIndex++;
				}
				//swipe up
				else if(Event->tfinger.dy * *Height > 0.25)
				{
					CursorIndex++;
					SelectedIndex++;
				}
				break;*/
				//Joycon button pressed
                case SDL_JOYBUTTONDOWN:
                    if (Event->jbutton.which == 0)
					{
						//Plus pressed
						if (Event->jbutton.button == 10)
						{
                            *IsDone = 1;
                        }
						//X pressed
						else if (Event->jbutton.button == 2)
						{
                            //Get info about the current status
							EmuEmulationStatus CurrentStatus;
							nfpemuGetStatus(&CurrentStatus);
							//Change Emuiibo status
							switch(CurrentStatus)
							{
								case 0:
								nfpemuSetEmulationOff();
								break;
								case 1:
								nfpemuSetEmulationOnForever();
								break;
								case 2:
								nfpemuSetEmulationOnOnce();
								break;
							}

                        }
						//Y pressed
						else if(Event->jbutton.button == 3)
						{
							nfpemuMoveToNextAmiibo(NULL);
						}
						//Up pressed
						else if(Event->jbutton.button == 13)
						{
							CursorIndex--;
							SelectedIndex--;
						}
						//Down pressed
						else if(Event->jbutton.button == 15)
						{
							CursorIndex++;
							SelectedIndex++;
						}
						//A pressed
						else if(Event->jbutton.button == 0)
						{
							char PathToAmiibo[FS_MAX_PATH] = "sdmc:/emuiibo/amiibo/";
							strcat(PathToAmiibo, Files.at(SelectedIndex).d_name);
							nfpemuSetCustomAmiibo(PathToAmiibo);
						}
						//B pressed so switch to Amiibo generator
						else if(Event->jbutton.button == 1)
						{
							*WindowState = 1;
						}
                    }
                    break;
            }
        }
		
	//Draw the BG
	SDL_SetRenderDrawColor(renderer, 94, 94, 94, 255);
	SDL_Rect BGRect = {0,0, *Width, *Height};
	SDL_RenderFillRect(renderer, &BGRect);
	
	//Draw the UI
	DrawHeader();
	DrawFooter();
	DrawList();
	
	//Reset touch coords
	TouchX = -1;
	TouchY = -1;
}

void AmiigoUI::DrawHeader()
{
	//Draw the header
	SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
	SDL_Rect HeaderRect = {0,0, *Width, HeaderHeight};
	SDL_RenderFillRect(renderer, &HeaderRect);
	//Get the Amiibo path
	char CurrentAmiibo[FS_MAX_PATH] = {0};
	nfpemuGetCurrentAmiibo(CurrentAmiibo, NULL);
	//Draw the Amiibo path text
	SDL_Surface* HeaderTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, CurrentAmiibo, TextColour, *Width);
	SDL_Texture* HeaderTextTexture = SDL_CreateTextureFromSurface(renderer, HeaderTextSurface);
	SDL_Rect HeaderTextRect = {(*Width - HeaderTextSurface->w) / 2, (HeaderHeight - HeaderTextSurface->h) / 2, HeaderTextSurface->w, HeaderTextSurface->h};
	SDL_RenderCopy(renderer, HeaderTextTexture, NULL, &HeaderTextRect);
	//Clean up
	SDL_DestroyTexture(HeaderTextTexture);
	SDL_FreeSurface(HeaderTextSurface);
	//Switch to next Amiibo
	if(CheckButtonPressed(&HeaderRect))
	{
		nfpemuMoveToNextAmiibo(NULL);
	}
}

void AmiigoUI::DrawFooter()
{
	//Get info about the current status
	EmuEmulationStatus CurrentStatus;
	nfpemuGetStatus(&CurrentStatus);
	//Draw the footer
	int FooterYOffset = *Height - FooterHeight;
	SDL_Rect FooterRect = {0,FooterYOffset, *Width, FooterHeight};
	string StatusText = "";
	switch(CurrentStatus)
	{
		case 0:
		StatusText = "On";
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		break;
		case 1:
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		StatusText = "Temporary on";
		break;
		case 2:
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		StatusText = "Off";
		break;
	}
	
	//Footer was pressed so we should change the status
	if(CheckButtonPressed(&FooterRect))
	{
		switch(CurrentStatus)
		{
			case 0:
			nfpemuSetEmulationOff();
			break;
			case 1:
			nfpemuSetEmulationOnForever();
			break;
			case 2:
			nfpemuSetEmulationOnOnce();
			break;
		}
	}
	
	SDL_RenderFillRect(renderer, &FooterRect);
	
	//Draw the status text
	SDL_Surface* FooterTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, StatusText.c_str(), TextColour, *Width);
	SDL_Texture* FooterTextTexture = SDL_CreateTextureFromSurface(renderer, FooterTextSurface);
	SDL_Rect FooterTextRect = {(*Width - FooterTextSurface->w) / 2, FooterYOffset + ((FooterHeight - FooterTextSurface->h) / 2), FooterTextSurface->w, FooterTextSurface->h};
	SDL_RenderCopy(renderer, FooterTextTexture, NULL, &FooterTextRect);
	//Clean up
	SDL_DestroyTexture(FooterTextTexture);
	SDL_FreeSurface(FooterTextSurface);
}

void AmiigoUI::DrawList()
{
	int ListingsOnScreen = 10;
	int ListingHeight = ListHeight / ListingsOnScreen;
	int ListLength = ListingsOnScreen;
	if(ListLength > Files.size()) ListLength = Files.size();
	
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
		SelectedIndex = Files.size()-1;
		CursorIndex = ListLength;
		ListRenderOffset = Files.size() - ListLength-1;
	}
	else if(SelectedIndex > Files.size()-1)
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
		
		SDL_Rect MenuItem = {0, HeaderHeight + (i * ListingHeight), *Width, ListingHeight};
		SDL_RenderFillRect(renderer, &MenuItem);
		
		//Draw file names
		SDL_Color TextColour = {0, 0, 0};
		SDL_Surface* FileNameSurface = TTF_RenderUTF8_Blended_Wrapped(ListFont, Files.at(i + ListRenderOffset).d_name, TextColour, *Width);
		SDL_Texture* FileNameTexture = SDL_CreateTextureFromSurface(renderer, FileNameSurface);
		SDL_Rect AmiiboNameRect = {0, MenuItem.y + ((MenuItem.h - FileNameSurface->h) / 2), FileNameSurface->w, FileNameSurface->h};
		SDL_RenderCopy(renderer, FileNameTexture, NULL, &AmiiboNameRect);
		
		//Check if option is pressed
		if(CheckButtonPressed(&MenuItem))
		{
			char PathToAmiibo[FS_MAX_PATH] = "sdmc:/emuiibo/amiibo/";
			strcat(PathToAmiibo, Files.at(i + ListRenderOffset).d_name);
			nfpemuSetCustomAmiibo(PathToAmiibo);
			int SelectedDifference = i - CursorIndex;
			SelectedIndex += SelectedDifference;
			CursorIndex = i;
		}
		
		//Clean up
		SDL_DestroyTexture(FileNameTexture);
		SDL_FreeSurface(FileNameSurface);
	}
}

void AmiigoUI::ScanForAmiibos()
{
	//clear the Amiibo list
	Files.clear();
	//Reset some vars so we don't crash when a new Amiibo is added
	SelectedIndex = 0;
	CursorIndex = 0;
	ListRenderOffset = 0;
	//Do the asctual scanning
	DIR* dir;
	struct dirent* ent;
	dir = opendir("sdmc:/emuiibo/amiibo/");
	while ((ent = readdir(dir)))
	{
		Files.push_back(*ent);
	}
	closedir(dir);
}

void AmiigoUI::PleaseWait()
{
	//Draw the rect
	SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
	SDL_Rect MessageRect = {0,0, *Width, *Height};
	SDL_RenderFillRect(renderer, &MessageRect);
	//Draw the please wait text
	SDL_Surface* MessageTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, "Please wait while we get data from the Amiibo API...", TextColour, *Width);
	SDL_Texture* MessagerTextTexture = SDL_CreateTextureFromSurface(renderer, MessageTextSurface);
	SDL_Rect HeaderTextRect = {(*Width - MessageTextSurface->w) / 2, (*Height - MessageTextSurface->h) / 2, MessageTextSurface->w, MessageTextSurface->h};
	SDL_RenderCopy(renderer, MessagerTextTexture, NULL, &HeaderTextRect);
	//Clean up
	SDL_DestroyTexture(MessagerTextTexture);
	SDL_FreeSurface(MessageTextSurface);
}

bool AmiigoUI::CheckButtonPressed(SDL_Rect* ButtonRect)
{
	if(TouchX > ButtonRect->x && TouchX < ButtonRect->x + ButtonRect->w && TouchY > ButtonRect->y && TouchY < ButtonRect->y + ButtonRect->h) return true;
	else return false;
}