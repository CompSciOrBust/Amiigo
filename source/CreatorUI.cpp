//Note this is extremely janky. I made it while tired.
//I should have probably made a common UI class for this and AmiigoUI but I didn't so there's a lot of reused code.
//It could definitely be more efficient. We only really need to call GetDataFromAPI() once but I'm lazy so I didn't.
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

CreatorUI::CreatorUI()
{
	nifmInitialize(); //Init nifm for connection stuff
	HeaderFont = TTF_OpenFont("romfs:/font.ttf", 48); //Load the header font
	ListFont = TTF_OpenFont("romfs:/font.ttf", 32); //Load the list font
	GetDataFromAPI(""); //Get data from the API
	
	//Get all of the Series' names
	for(int i = 0; i < JDataSize; i++)
	{
		bool IsInVec = false;
		string SeriesName = JData["amiibo"][i]["amiiboSeries"].get<std::string>();
		//Loop through every element in the vector
		for(int j = 0; j < SeriesVec.size(); j++)
		{
			//If the vector has the name we break the loop
			if(SeriesVec.at(j) == SeriesName)
			{
				IsInVec = true;
				break;
			}
		}
		if(!IsInVec)
		{
			SeriesVec.push_back(JData["amiibo"][i]["amiiboSeries"].get<std::string>());
		}
	}
}

void CreatorUI::GetDataFromAPI(string FilterTerm)
{
	//Note we need to poke around NIFM to check that the user actually has an internet connection otherwise we'll crashes
	string APIURI = "https://www.amiiboapi.com/api/amiibo" + FilterTerm;
	AmiiboAPIString = RetrieveContent(APIURI, "application/json").c_str();
	JData = json::parse(AmiiboAPIString);
	JDataSize = JData["amiibo"].size();
}

void CreatorUI::DrawUI()
{
	//This crashes when in the constructor for some reason
	HeaderHeight = (*Height / 100) * 10;
	ListHeight = *Height - HeaderHeight;
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
				//Joycon button pressed
                case SDL_JOYBUTTONDOWN:
                    if (Event->jbutton.which == 0)
					{
						//Plus pressed
						if (Event->jbutton.button == 10)
						{
                            *IsDone = 1;
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
							if(HasSelectedSeries)
							{
						    	string AmiiboPath = "sdmc:/emuiibo/amiibo/" + JData["amiibo"][SelectedIndex]["name"].get<std::string>();
						    	mkdir(AmiiboPath.c_str(), 0);
						    	//Write common.json
						    	string FilePath = AmiiboPath + "/common.json";
						    	ofstream CommonFileWriter(FilePath.c_str());
						    	CommonFileWriter << "{\"lastWriteDate\": \"2019-01-01\",\"writeCounter\": 0,\"version\": 0}";
						    	CommonFileWriter.close();
						    	//Write model.json
						    	FilePath = AmiiboPath + "/model.json";
						    	ofstream ModelFileWriter(FilePath.c_str());
						    	ModelFileWriter << "{\"amiiboId\": \"" + JData["amiibo"][SelectedIndex]["head"].get<std::string>() + JData["amiibo"][SelectedIndex]["tail"].get<std::string>() + "\"}";
						    	ModelFileWriter.close();
						    	//write tag.json
						    	FilePath = AmiiboPath + "/tag.json";
						    	ofstream TagFileWriter(FilePath.c_str());
						    	TagFileWriter << "{\"randomUuid\": true}";
						    	TagFileWriter.close();
						    	//write register.json
						    	FilePath = AmiiboPath + "/register.json";
						    	ofstream RegFileWriter(FilePath.c_str());
						    	RegFileWriter << "{\"name\": \"" + JData["amiibo"][SelectedIndex]["name"].get<std::string>() + "\",\"firstWriteDate\": \"2019-01-01\",\"miiCharInfo\": \"mii-charinfo.bin\"}";
						    	RegFileWriter.close();
							}
							else
							{
								GetDataFromAPI("/?amiiboSeries=" + FormatURL(SeriesVec.at(SelectedIndex)));
								//Reset some vars so we don't crash
								SelectedIndex = 0;
								CursorIndex = 0;
								ListRenderOffset = 0;
								HasSelectedSeries = true;
							}
						}
						//B pressed
						else if(Event->jbutton.button == 1)
						{
							if(HasSelectedSeries)
							{
								//Reset some vars so we don't crash
								SelectedIndex = 0;
								CursorIndex = 0;
								ListRenderOffset = 0;
								HasSelectedSeries = false;
							}
							else *WindowState = 0;
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
	if(HasSelectedSeries)
	{
		DrawAmiiboList();
	}
	else
	{
		DrawSeriesList();
	}
	
	//Reset touch coords
	TouchX = -1;
	TouchY = -1;
}

void CreatorUI::DrawAmiiboList()
{
	int ListingsOnScreen = 11;
	int ListingHeight = ListHeight / ListingsOnScreen;
	int ListLength = ListingsOnScreen;
	if(ListLength > JDataSize) ListLength = JDataSize;
	
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
		SelectedIndex = JDataSize-1;
		CursorIndex = ListLength;
		ListRenderOffset = JDataSize - ListLength-1;
	}
	else if(SelectedIndex > JDataSize-1)
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
		string AmiiboText = JData["amiibo"][i + ListRenderOffset]["name"].get<std::string>() + " - " + JData["amiibo"][i + ListRenderOffset]["amiiboSeries"].get<std::string>();
		SDL_Surface* FileNameSurface = TTF_RenderUTF8_Blended_Wrapped(ListFont, AmiiboText.c_str(), TextColour, *Width);
		SDL_Texture* FileNameTexture = SDL_CreateTextureFromSurface(renderer, FileNameSurface);
		SDL_Rect AmiiboNameRect = {0, MenuItem.y + ((MenuItem.h - FileNameSurface->h) / 2), FileNameSurface->w, FileNameSurface->h};
		SDL_RenderCopy(renderer, FileNameTexture, NULL, &AmiiboNameRect);
		
		//Check if option is pressed
		if(CheckButtonPressed(&MenuItem))
		{
			
		}
		
		//Clean up
		SDL_DestroyTexture(FileNameTexture);
		SDL_FreeSurface(FileNameSurface);
	}
}

void CreatorUI::DrawSeriesList()
{
	int ListingsOnScreen = 11;
	int ListingHeight = ListHeight / ListingsOnScreen;
	int ListLength = ListingsOnScreen;
	
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
		SelectedIndex = SeriesVec.size()-1;
		CursorIndex = ListLength;
		ListRenderOffset = SeriesVec.size() - ListLength-1;
	}
	else if(SelectedIndex > SeriesVec.size()-1)
	{
		SelectedIndex = 0;
		CursorIndex = 0;
		ListRenderOffset = 0;
	}
	
	//Draw the Series list
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
		string AmiiboText = SeriesVec.at(i + ListRenderOffset);
		SDL_Surface* FileNameSurface = TTF_RenderUTF8_Blended_Wrapped(ListFont, AmiiboText.c_str(), TextColour, *Width);
		SDL_Texture* FileNameTexture = SDL_CreateTextureFromSurface(renderer, FileNameSurface);
		SDL_Rect AmiiboNameRect = {0, MenuItem.y + ((MenuItem.h - FileNameSurface->h) / 2), FileNameSurface->w, FileNameSurface->h};
		SDL_RenderCopy(renderer, FileNameTexture, NULL, &AmiiboNameRect);
		
		//Check if option is pressed
		if(CheckButtonPressed(&MenuItem))
		{
			
		}
		
		//Clean up
		SDL_DestroyTexture(FileNameTexture);
		SDL_FreeSurface(FileNameSurface);
	}
}

void CreatorUI::DrawHeader()
{
	//Draw the header
	SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
	SDL_Rect HeaderRect = {0,0, *Width, HeaderHeight};
	SDL_RenderFillRect(renderer, &HeaderRect);
	//Draw the Amiibo path text
	SDL_Surface* HeaderTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, "Amiigo Maker", TextColour, *Width);
	SDL_Texture* HeaderTextTexture = SDL_CreateTextureFromSurface(renderer, HeaderTextSurface);
	SDL_Rect HeaderTextRect = {(*Width - HeaderTextSurface->w) / 2, (HeaderHeight - HeaderTextSurface->h) / 2, HeaderTextSurface->w, HeaderTextSurface->h};
	SDL_RenderCopy(renderer, HeaderTextTexture, NULL, &HeaderTextRect);
	//Clean up
	SDL_DestroyTexture(HeaderTextTexture);
	SDL_FreeSurface(HeaderTextSurface);
}

string CreatorUI::GetKBInput()
{
	//Init swkbd
	Result rc=0;
	SwkbdConfig kbd;
	char tmpoutstr[16] = {0};
	rc = swkbdCreate(&kbd, 0);
	if (R_SUCCEEDED(rc)) 
	{
		// Select a Preset to use, if any.
		swkbdConfigMakePresetDefault(&kbd);
		printf("Running swkbdShow...\n");
		rc = swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
		printf("swkbdShow(): 0x%x\n", rc);

		if (R_SUCCEEDED(rc))
		{
			printf("out str: %s\n", tmpoutstr);
		}
		swkbdClose(&kbd);
	}
	return tmpoutstr;
}

bool CreatorUI::CheckButtonPressed(SDL_Rect* ButtonRect)
{
	if(TouchX > ButtonRect->x && TouchX < ButtonRect->x + ButtonRect->w && TouchY > ButtonRect->y && TouchY < ButtonRect->y + ButtonRect->h) return true;
	else return false;
}