#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <nfpemu.h>
#include <vector>
#include <dirent.h>
#include <UI.h>
#include <Utils.h>
#include "nlohmann/json.hpp"
#include <fstream>
using namespace std;
using json = nlohmann::json;

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
	json JData;
	public:
	AmiigoUI();
	void GetInput();
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
	ScrollList *MenuList;
	int AmiiboListWidth;
	string ListDir = "sdmc:/emuiibo/amiibo/";
};

AmiigoUI::AmiigoUI()
{
	//Load the header font
	//HeaderFont = TTF_OpenFont("romfs:/font.ttf", 48);
	HeaderFont = GetSharedFont(48);
	
	//Create the lists
	AmiiboList = new ScrollList();
	MenuList = new ScrollList();
	//Add items to the menu list
	MenuList->ListingTextVec.push_back("Amiibo list");
	MenuList->ListingTextVec.push_back("Amiigo Maker");
	MenuList->ListingTextVec.push_back("Check for updates");
	MenuList->ListingTextVec.push_back("Exit");
	//Scan the Amiibo folder for Amiibos
	ScanForAmiibos();
}

void AmiigoUI::GetInput()
{
	//Scan input
	while (SDL_PollEvent(Event))
		{
            switch (Event->type)
			{
				//Touchscreen
				case SDL_FINGERDOWN:
				TouchX = Event->tfinger.x * *Width;
				TouchY = Event->tfinger.y * *Height;
				//Set the touch list pointers because we need them to work in both menus
				MenuList->TouchListX = &TouchX;
				MenuList->TouchListY = &TouchY;
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
							if(AmiiboList->IsActive)
							{
								AmiiboList->CursorIndex--;
								AmiiboList->SelectedIndex--;
							}
							else
							{
								MenuList->CursorIndex--;
								MenuList->SelectedIndex--;
							}
						}
						//Down pressed
						else if(Event->jbutton.button == 15)
						{
							if(AmiiboList->IsActive)
							{
								AmiiboList->CursorIndex++;
								AmiiboList->SelectedIndex++;
							}
							else
							{
								MenuList->CursorIndex++;
								MenuList->SelectedIndex++;
							}
						}
						//Left or right pressed
						else if(Event->jbutton.button == 12 || Event->jbutton.button == 14)
						{
							MenuList->IsActive = AmiiboList->IsActive;
							AmiiboList->IsActive = !AmiiboList->IsActive;
						}
						//A pressed
						else if(Event->jbutton.button == 0)
						{
							if(AmiiboList->IsActive)
							{
								SetAmiibo(AmiiboList->SelectedIndex);
							}
							else
							{
								*WindowState = MenuList->SelectedIndex;
							}
						}
						//B pressed
						else if(Event->jbutton.button == 1)
						{
							ListDir = GoUpDir(ListDir);
							ScanForAmiibos();
						}
						//Left stick pressed
						else if(Event->jbutton.button == 4)
						{
							//Delete Amiibo. This is temporary until I have time to implement a proper menu for deleting and renaming
							char PathToAmiibo[FS_MAX_PATH] = ""; //Without assigning we get a random char. Why?
							strcat(PathToAmiibo, ListDir.c_str());
							strcat(PathToAmiibo, Files.at(AmiiboList->SelectedIndex).d_name);
							fsdevDeleteDirectoryRecursively(PathToAmiibo);
							ScanForAmiibos();
						}
                    }
                    break;
            }
        }
		
	//Check if list item selected via touch screen
	if(AmiiboList->ItemSelected)
	{
		SetAmiibo(AmiiboList->SelectedIndex);
		MenuList->IsActive = false;
		AmiiboList->IsActive = true;
	}
	else if(MenuList->ItemSelected)
	{
		*WindowState = MenuList->SelectedIndex;
		MenuList->IsActive = true;
		AmiiboList->IsActive = false;
	}
}

void AmiigoUI::DrawUI()
{		
	//Draw the BG
	SDL_SetRenderDrawColor(renderer, 94, 94, 94, 255);
	SDL_Rect BGRect = {0,0, *Width, *Height};
	SDL_RenderFillRect(renderer, &BGRect);
	
	//Draw the UI
	DrawHeader();
	DrawFooter();
	AmiiboList->DrawList();
	MenuList->DrawList();
	DrawButtonBorders(renderer, AmiiboList, MenuList, HeaderHeight, FooterHeight, *Width, *Height, false);
	
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
	string HeaderText = "";
	nfpemuGetCurrentAmiibo(CurrentAmiibo, NULL);
	//String is empty so we need to set it to something so SDL doesn't crash
	if(CurrentAmiibo[0] == NULL)
	{
		HeaderText = "No Amiibo Selected";
	}
	//Get the Amiibo name from the json
	else
	{
		//Append the register path to the current amiibo var
		strcat(CurrentAmiibo, "/register.json");
		string FileContents = "";
		ifstream FileReader(CurrentAmiibo);
		//If the register file doesn't exist display message. This prevents a infinate loop.
		if(!FileReader) HeaderText = "Missing register json!";
		else //Else get the amiibo name from the json
		{
			//Read each line
			for(int i = 0; !FileReader.eof(); i++)
			{
				string TempLine = "";
				getline(FileReader, TempLine);
				FileContents += TempLine;
			}
			//Parse the data and set the HeaderText var
			JData = json::parse(FileContents);
			HeaderText = JData["name"].get<std::string>();
		}
	}
	//Draw the Amiibo path text
	SDL_Surface* HeaderTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, HeaderText.c_str(), TextColour, *Width);
	SDL_Texture* HeaderTextTexture = SDL_CreateTextureFromSurface(renderer, HeaderTextSurface);
	SDL_Rect HeaderTextRect = {(*Width - HeaderTextSurface->w) / 2, (HeaderHeight - HeaderTextSurface->h) / 2, HeaderTextSurface->w, HeaderTextSurface->h};
	SDL_RenderCopy(renderer, HeaderTextTexture, NULL, &HeaderTextRect);
	//Clean up
	SDL_DestroyTexture(HeaderTextTexture);
	SDL_FreeSurface(HeaderTextSurface);
	//Switch to next Amiibo
	if(CheckButtonPressed(&HeaderRect, TouchX, TouchY))
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
	if(CheckButtonPressed(&FooterRect, TouchX, TouchY))
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

void AmiigoUI::ScanForAmiibos()
{
	//clear the Amiibo list
	Files.clear();
	//Reset some vars so we don't crash when a new Amiibo is added
	AmiiboList->SelectedIndex = 0;
	AmiiboList->CursorIndex = 0;
	AmiiboList->ListRenderOffset = 0;
	//Do the actual scanning
	DIR* dir;
	struct dirent* ent;
	dir = opendir(ListDir.c_str());
	while ((ent = readdir(dir)))
	{
		Files.push_back(*ent);
	}
	closedir(dir);
	
	AmiiboList->ListingTextVec.clear();
	for(int i = 0; i < Files.size(); i++)
	{
		AmiiboList->ListingTextVec.push_back(Files.at(i).d_name);
	}
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

void AmiigoUI::SetAmiibo(int Index)
{
	char PathToAmiibo[FS_MAX_PATH] = "";
	strcat(PathToAmiibo, ListDir.c_str());
	strcat(PathToAmiibo, Files.at(Index).d_name);
	//Check if Amiibo or empty folder
	string TagPath = PathToAmiibo;
	TagPath += "/tag.json";
	if(CheckFileExists(TagPath))
	{
		ListDir = PathToAmiibo;
		ListDir += "/";
		ScanForAmiibos();
	}
	else nfpemuSetCustomAmiibo(PathToAmiibo);
}

void AmiigoUI::InitList()
{
	//This crashes when in the constructor because these values aren't set yet
	HeaderHeight = (*Height / 100) * 10;
	FooterHeight = (*Height / 100) * 10;
	AmiiboListWidth = (*Width / 100) * 80;
	//for shared font
	PlFontData standardFontData;
	plGetSharedFontByType(&standardFontData, PlSharedFontType_Standard);
	
	//Assign vars
	AmiiboList->TouchListX = &TouchX;
	AmiiboList->TouchListY = &TouchY;
	AmiiboList->ListFont = GetSharedFont(32); //Load the list font
	AmiiboList->ListingsOnScreen = 10;
	AmiiboList->ListHeight = *Height - HeaderHeight - FooterHeight;
	AmiiboList->ListWidth = AmiiboListWidth;
	AmiiboList->ListYOffset = HeaderHeight;
	AmiiboList->renderer = renderer;
	AmiiboList->IsActive = true;
	/*
	for(int i = 0; i < Files.size(); i++)
	{
		AmiiboList->ListingTextVec.push_back(Files.at(i).d_name);
	}*/
	//Menu list
	MenuList->TouchListX = &TouchX;
	MenuList->TouchListY = &TouchY;
	MenuList->ListFont = GetSharedFont(32); //Load the list font
	MenuList->ListingsOnScreen = MenuList->ListingTextVec.size();
	MenuList->ListHeight = *Height - HeaderHeight - FooterHeight;
	MenuList->ListWidth = *Width - AmiiboListWidth;
	MenuList->ListYOffset = HeaderHeight;
	MenuList->ListXOffset = AmiiboListWidth;
	MenuList->renderer = renderer;
	MenuList->CenterText = true;
}