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
#include <UI.h>
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
	int HeaderHeight;
	int FooterHeight;
	int ListHeight;
	bool CheckButtonPressed(SDL_Rect*);
	int TouchX = -1;
	int TouchY = -1;
	json JData;
	int JDataSize = 0;
	bool HasSelectedSeries = false;
	vector<string> SeriesVec;
	vector<AmiiboVars> AmiiboVarsVec;
	vector<AmiiboVars> SortedAmiiboVarsVec;
	string AmiiboAPIString = "";
	public:
	CreatorUI();
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
};

CreatorUI::CreatorUI()
{
	nifmInitialize(); //Init nifm for connection stuff
	HeaderFont = TTF_OpenFont("romfs:/font.ttf", 48); //Load the header font
	ListFont = TTF_OpenFont("romfs:/font.ttf", 32); //Load the list font
	GetDataFromAPI(""); //Get data from the API
	
	//Create the lists
	SeriesList = new ScrollList();
	//Get all of the Series' names and add Amiibos to the AmiiboVarsVec
	for(int i = 0; i < JDataSize; i++)
	{
		bool IsInVec = false;
		string SeriesName = JData["amiibo"][i]["amiiboSeries"].get<std::string>();
		
		//Add data to the AmiiboVarsVec
		AmiiboVars TempAmiiboVars;
		TempAmiiboVars.AmiiboSeries = SeriesName;
		TempAmiiboVars.AmiiboName = JData["amiibo"][i]["name"].get<std::string>();
		TempAmiiboVars.ListIndex = i;
		AmiiboVarsVec.push_back(TempAmiiboVars);
		
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

void CreatorUI::InitList()
{
	//Create the lists
	SeriesList->TouchListX = &TouchX;
	SeriesList->TouchListY = &TouchY;
	SeriesList->ListFont = TTF_OpenFont("romfs:/font.ttf", 32); //Load the list font
	SeriesList->ListingsOnScreen = 11;
	SeriesList->ListWidth = *Width;
	SeriesList->renderer = renderer;
	SeriesList->ListingTextVec = SeriesVec;
}

void CreatorUI::DrawUI()
{
	//This crashes when in the constructor for some reason
	HeaderHeight = (*Height / 100) * 10;
	ListHeight = *Height - HeaderHeight;
	SeriesList->ListHeight = *Height - HeaderHeight;
	SeriesList->ListYOffset = HeaderHeight;
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
							SeriesList->CursorIndex--;
							SeriesList->SelectedIndex--;
						}
						//Down pressed
						else if(Event->jbutton.button == 15)
						{
							SeriesList->CursorIndex++;
							SeriesList->SelectedIndex++;
						}
						//A pressed
						else if(Event->jbutton.button == 0)
						{
							ListSelect();
						}
						//B pressed
						else if(Event->jbutton.button == 1)
						{
							if(HasSelectedSeries)
							{
								//Reset some vars so we don't crash
								SeriesList->ListingTextVec = SeriesVec;
								SeriesList->SelectedIndex = 0;
								SeriesList->CursorIndex = 0;
								SeriesList->ListRenderOffset = 0;
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
	SeriesList->DrawList();
	
	//Reset touch coords
	TouchX = -1;
	TouchY = -1;
}

void CreatorUI::ListSelect()
{
	//Create the virtual amiibo on the SD card
	if(HasSelectedSeries)
	{
		int IndexInJdata = SortedAmiiboVarsVec.at(SeriesList->SelectedIndex).ListIndex;
        string AmiiboPath = "sdmc:/emuiibo/amiibo/" + JData["amiibo"][IndexInJdata]["name"].get<std::string>();
        mkdir(AmiiboPath.c_str(), 0);
        //Write common.json
        string FilePath = AmiiboPath + "/common.json";
        ofstream CommonFileWriter(FilePath.c_str());
        CommonFileWriter << "{\"lastWriteDate\": \"2019-01-01\",\"writeCounter\": 0,\"version\": 0}";
        CommonFileWriter.close();
        //Write model.json
        FilePath = AmiiboPath + "/model.json";
        ofstream ModelFileWriter(FilePath.c_str());
        ModelFileWriter << "{\"amiiboId\": \"" + JData["amiibo"][IndexInJdata]["head"].get<std::string>() + JData["amiibo"][IndexInJdata]["tail"].get<std::string>() + "\"}";
        ModelFileWriter.close();
        //write tag.json
        FilePath = AmiiboPath + "/tag.json";
        ofstream TagFileWriter(FilePath.c_str());
        TagFileWriter << "{\"randomUuid\": true}";
        TagFileWriter.close();
        //write register.json
        FilePath = AmiiboPath + "/register.json";
        ofstream RegFileWriter(FilePath.c_str());
        RegFileWriter << "{\"name\": \"" + JData["amiibo"][IndexInJdata]["name"].get<std::string>() + "\",\"firstWriteDate\": \"2019-01-01\",\"miiCharInfo\": \"mii-charinfo.bin\"}";
        RegFileWriter.close();
	}
	//Add the Amiibos from the selected series to the list
	else
	{
		HasSelectedSeries = true;
		string SelectedSeries = SeriesVec.at(SeriesList->SelectedIndex);
		SeriesList->ListingTextVec.clear();
		SortedAmiiboVarsVec.clear();
		for(int i = 0; i < AmiiboVarsVec.size(); i++)
		{
			//There's something happening here
			//What it is ain't exactly clear
			//There's a class with a bug over there
			//When using it we should beware
			if(AmiiboVarsVec.at(i).AmiiboSeries == SelectedSeries)
			{
				SortedAmiiboVarsVec.push_back(AmiiboVarsVec.at(i));
				SeriesList->ListingTextVec.push_back(AmiiboVarsVec.at(i).AmiiboName);
				//SeriesList->ListingTextVec.push_back(SortedAmiiboVarsVec.at(SortedAmiiboVarsVec.size()-1).AmiiboName);
			}
		}
		//Reset some vars so we don't crash
		SeriesList->SelectedIndex = 0;
		SeriesList->CursorIndex = 0;
		SeriesList->ListRenderOffset = 0;
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

void CreatorUI::GetDataFromAPI(string FilterTerm)
{
	//Note we need to poke around NIFM to check that the user actually has an internet connection otherwise we'll crashes
	string APIURI = "https://www.amiiboapi.com/api/amiibo" + FilterTerm;
	AmiiboAPIString = RetrieveContent(APIURI, "application/json").c_str();
	JData = json::parse(AmiiboAPIString);
	JDataSize = JData["amiibo"].size();
}

bool CreatorUI::CheckButtonPressed(SDL_Rect* ButtonRect)
{
	if(TouchX > ButtonRect->x && TouchX < ButtonRect->x + ButtonRect->w && TouchY > ButtonRect->y && TouchY < ButtonRect->y + ButtonRect->h) return true;
	else return false;
}