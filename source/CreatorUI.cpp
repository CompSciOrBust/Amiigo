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
#include "Utils.h"

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

CreatorUI::CreatorUI()
{
	nifmInitialize(NifmServiceType_User); //Init nifm for connection stuff
	HeaderFont = GetSharedFont(48);
	ListFont = GetSharedFont(32);
	//HeaderFont = TTF_OpenFont("romfs:/font.ttf", 48); //Load the header font
	//ListFont = TTF_OpenFont("romfs:/font.ttf", 32); //Load the list font
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
		for(u32 j = 0; j < SeriesVec.size(); j++)
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
	SeriesList->ListFont = GetSharedFont(32); //Load the list font
	SeriesList->ListingsOnScreen = 10;
	SeriesList->ListWidth = SeriesListWidth;
	SeriesList->renderer = renderer;
	SeriesList->ListingTextVec = SeriesVec;
}

void CreatorUI::GetInput()
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
							if(SeriesList->IsActive)
							{
								SeriesList->CursorIndex--;
								SeriesList->SelectedIndex--;
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
							if(SeriesList->IsActive)
							{
								SeriesList->CursorIndex++;
								SeriesList->SelectedIndex++;
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
							MenuList->IsActive = SeriesList->IsActive;
							SeriesList->IsActive = !SeriesList->IsActive;
						}
						//A pressed
						else if(Event->jbutton.button == 0)
						{
							if(SeriesList->IsActive)
							{
								ListSelect();
							}
							else
							{
								*WindowState = MenuList->SelectedIndex;
							}
						}
						//B pressed
						else if(Event->jbutton.button == 1)
						{
							//Reset some vars so we don't crash
							SeriesList->ListingTextVec = SeriesVec;
							SeriesList->SelectedIndex = 0;
							SeriesList->CursorIndex = 0;
							SeriesList->ListRenderOffset = 0;
							HasSelectedSeries = false;
						}
                    }
                    break;
            }
        }
}

void CreatorUI::DrawUI()
{
	//This crashes when in the constructor for some reason
	HeaderHeight = (*Height / 100) * 10;
	FooterHeight = (*Height / 100) * 10;
	ListHeight = *Height - HeaderHeight;
	SeriesList->ListHeight = *Height - HeaderHeight - FooterHeight;
	SeriesList->ListYOffset = HeaderHeight;
		
	//Draw the BG
	SDL_SetRenderDrawColor(renderer, 94, 94, 94, 255);
	SDL_Rect BGRect = {0,0, *Width, *Height};
	SDL_RenderFillRect(renderer, &BGRect);
	
	//Draw the UI
	DrawHeader();
	SeriesList->DrawList();
	MenuList->DrawList();
	DrawFooter();
	DrawButtonBorders(renderer, SeriesList, MenuList, HeaderHeight, FooterHeight, *Width, *Height, true);
	//Check if list item selected via touch screen
	if(SeriesList->ItemSelected)
	{
		MenuList->IsActive = false;
		SeriesList->IsActive = true;
	}
	else if(MenuList->ItemSelected)
	{
		*WindowState = MenuList->SelectedIndex;
		MenuList->IsActive = true;
		SeriesList->IsActive = false;
	}
	
	//Reset touch coords
	TouchX = -1;
	TouchY = -1;
}

void CreatorUI::ListSelect()
{
	//Create the virtual amiibo on the SD card
	if(HasSelectedSeries)
	{
		PleaseWait("Please wait, building amiibo...");
		int IndexInJdata = SortedAmiiboVarsVec.at(SeriesList->SelectedIndex).ListIndex;
        string AmiiboPath = *CurrentPath + JData["amiibo"][IndexInJdata]["name"].get<std::string>();
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
		
		//create icon
		mkdir("sdmc:/config/amiigo/IMG/", 0);
		string iconname = "sdmc:/config/amiigo/IMG/"+JData["amiibo"][IndexInJdata]["head"].get<std::string>()+JData["amiibo"][IndexInJdata]["tail"].get<std::string>()+".png";
		if(!CheckFileExists(iconname))
		RetrieveToFile(JData["amiibo"][IndexInJdata]["image"].get<std::string>(), iconname);
	}
	//Add the Amiibos from the selected series to the list
	else
	{


		HasSelectedSeries = true;
		string SelectedSeries = SeriesVec.at(SeriesList->SelectedIndex);
		SeriesList->ListingTextVec.clear();
		SortedAmiiboVarsVec.clear();
		for(u32 i = 0; i < AmiiboVarsVec.size(); i++)
		{
			//There's something happening here
			//What it is ain't exactly clear
			//There's a class with a bug over there
			//When using it we should beware
			if(AmiiboVarsVec.at(i).AmiiboSeries == SelectedSeries)
			{
				SortedAmiiboVarsVec.push_back(AmiiboVarsVec.at(i));
				if(CheckFileExists(*CurrentPath + AmiiboVarsVec.at(i).AmiiboName +"/tag.json"))
					SeriesList->ListingTextVec.push_back(AmiiboVarsVec.at(i).AmiiboName+" *");
					else
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
	//Make the Amiigo config dir

		while(true)//wait for the download of the api
		{
		ifstream DataFileReader("sdmc:/config/amiigo/API.json");
		getline(DataFileReader, AmiiboAPIString);
		DataFileReader.close();		
		if(AmiiboAPIString.size()!=0) break;			
		if(!HasConnection()) break;
		}
	if(json::accept(AmiiboAPIString))
	{
	//Parse and use the JSON data
	JData = json::parse(AmiiboAPIString);
	JDataSize = JData["amiibo"].size();
			
	}
}

void CreatorUI::DrawFooter()
{
	//Draw the select footer button
	int FooterYOffset = *Height - FooterHeight;
	SDL_Rect SelectFooterRect = {0,FooterYOffset, *Width/2, FooterHeight};
	string FooterText = "Select";
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	
	//Select was pressed
	if(CheckButtonPressed(&SelectFooterRect, TouchX, TouchY))
	{
		if(SeriesList->IsActive)
		{
			ListSelect();
		}
		else
		{
			*WindowState = MenuList->SelectedIndex;
		}
	}
	
	SDL_RenderFillRect(renderer, &SelectFooterRect);
	
	//Draw the text
	SDL_Surface* SelectTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, FooterText.c_str(), TextColour, SelectFooterRect.w);
	SDL_Texture* SelectTextTexture = SDL_CreateTextureFromSurface(renderer, SelectTextSurface);
	SDL_Rect FooterTextRect = {(SelectFooterRect.w - SelectTextSurface->w) / 2, FooterYOffset + ((FooterHeight - SelectTextSurface->h) / 2), SelectTextSurface->w, SelectTextSurface->h};
	SDL_RenderCopy(renderer, SelectTextTexture, NULL, &FooterTextRect);
	//Clean up
	SDL_DestroyTexture(SelectTextTexture);
	SDL_FreeSurface(SelectTextSurface);
	
	//Draw the back footer button
	SDL_Rect BackFooterRect = {*Width/2,FooterYOffset, *Width/2, FooterHeight};
	FooterText = "Back";
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	
	//Back was pressed
	if(CheckButtonPressed(&BackFooterRect, TouchX, TouchY))
	{
		//Reset some vars so we don't crash
		SeriesList->ListingTextVec = SeriesVec;
		SeriesList->SelectedIndex = 0;
		SeriesList->CursorIndex = 0;
		SeriesList->ListRenderOffset = 0;
		HasSelectedSeries = false;
	}
	
	SDL_RenderFillRect(renderer, &BackFooterRect);
	
	//Draw the status text
	SDL_Surface* BackTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, FooterText.c_str(), TextColour, BackFooterRect.w);
	SDL_Texture* BackTextTexture = SDL_CreateTextureFromSurface(renderer, BackTextSurface);
	SDL_Rect BackTextRect = {BackFooterRect.x + (BackFooterRect.w - BackTextSurface->w) / 2, FooterYOffset + ((FooterHeight - BackTextSurface->h) / 2), BackTextSurface->w, BackTextSurface->h};
	SDL_RenderCopy(renderer, BackTextTexture, NULL, &BackTextRect);
	//Clean up
	SDL_DestroyTexture(BackTextTexture);
	SDL_FreeSurface(BackTextSurface);
}

void CreatorUI::PleaseWait(string mensage)
{
	//Draw the rect
	SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
	SDL_Rect MessageRect = {0,0, *Width, *Height};
	SDL_RenderFillRect(renderer, &MessageRect);
	//Draw the please wait text
	SDL_Surface* MessageTextSurface = TTF_RenderUTF8_Blended_Wrapped(HeaderFont, mensage.c_str(), TextColour, *Width);
	SDL_Texture* MessagerTextTexture = SDL_CreateTextureFromSurface(renderer, MessageTextSurface);
	SDL_Rect HeaderTextRect = {(*Width - MessageTextSurface->w) / 2, (*Height - MessageTextSurface->h) / 2, MessageTextSurface->w, MessageTextSurface->h};
	SDL_RenderCopy(renderer, MessagerTextTexture, NULL, &HeaderTextRect);
	//Clean up
	SDL_DestroyTexture(MessagerTextTexture);
	SDL_FreeSurface(MessageTextSurface);

	SDL_RenderPresent(renderer);
}
