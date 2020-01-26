#include <iostream>
#include <string>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include "nlohmann/json.hpp"
#include <switch.h>
#include <vector>
#include <stdio.h>
#include <chrono>
#include "Utils.h"
using json = nlohmann::json;
string IDContents;

using namespace std;

int fsize(string fil) {
 std::streampos fsize = 0;

    std::ifstream myfile (fil, ios::in);  // File is of type const char*

    fsize = myfile.tellg();         // The file pointer is currently at the beginning
    myfile.seekg(0, ios::end);      // Place the file pointer at the end of file

    fsize = myfile.tellg() - fsize;
    myfile.close();

    static_assert(sizeof(fsize) >= sizeof(long long), "Oops.");
    return fsize;
}

bool CheckFileExists(string Path)
{
	struct stat Buffer;   
	return (stat (Path.c_str(), &Buffer) == 0); 
}

string GoUpDir(string Path)
{
	char CurrentPath[Path.length()] = "";
	strcat(CurrentPath, Path.c_str());
	CurrentPath[Path.length()-1] = 0x00;
	Path = CurrentPath;
	int LastSlash = Path.find_last_of('/');
	CurrentPath[LastSlash] = 0x00;
	if(strlen(CurrentPath) < 21)
	{
		return "sdmc:/emuiibo/amiibo/";
	}
	return CurrentPath;
}

/*
* copy function
*/
bool copy_me(string origen, string destino) {
    if(CheckFileExists(origen))
	{
		ifstream source(origen, ios::binary);
		ofstream dest(destino, ios::binary);
		dest << source.rdbuf();
		source.close();
		dest.close();
		return true;
	}else{
		return false;
	}
return 0;
}

void DrawJsonColorConfig(SDL_Renderer* renderer, string Head)
{
	if(CheckFileExists("sdmc:/config/amiigo/config.json"))
	{
		json JEData;
		if (IDContents.size() == 0)
		{
			ifstream IDReader("sdmc:/config/amiigo/config.json");
				//Read each line
				printf("Read Json\n");
				for(int i = 0; !IDReader.eof(); i++)
				{
					string TempLine = "";
					getline(IDReader, TempLine);
					IDContents += TempLine;
					printf("%s\n", TempLine.c_str());
				}
			IDReader.close();
		}
			
		if(json::accept(IDContents))
		{
			JEData = json::parse(IDContents);
			int CR = std::stoi(JEData[Head+"_R"].get<std::string>());
			int CG = std::stoi(JEData[Head+"_G"].get<std::string>());
			int CB = std::stoi(JEData[Head+"_B"].get<std::string>());
			int CA = std::stoi(JEData[Head+"_A"].get<std::string>());
			SDL_SetRenderDrawColor(renderer,CR,CG,CB,CA);
		}else{
			//remove bad config
			IDContents = "";
			remove("sdmc:/config/amiigo/bad_config.json");
			rename("sdmc:/config/amiigo/config.json","sdmc:/config/amiigo/bad_config.json");
		}
	}else{
		//Default values
		if(Head == "UI_borders") SDL_SetRenderDrawColor(renderer,0 ,0 ,0 ,255);
		if(Head == "UI_borders_list") SDL_SetRenderDrawColor(renderer,0 ,0 ,0 ,255);
		if(Head == "UI_background") SDL_SetRenderDrawColor(renderer,136 ,254 ,254 ,255);
		if(Head == "UI_background_alt") SDL_SetRenderDrawColor(renderer,0 ,178 ,212 ,255);
		if(Head == "UI_cursor") SDL_SetRenderDrawColor(renderer,255 ,255 ,255 ,255);
		if(Head == "AmiigoUI_DrawUI") SDL_SetRenderDrawColor(renderer,94 ,94 ,94 ,255);
		if(Head == "AmiigoUI_DrawHeader") SDL_SetRenderDrawColor(renderer,0 ,188 ,212 ,255);
		if(Head == "AmiigoUI_PleaseWait") SDL_SetRenderDrawColor(renderer,0 ,188 ,212 ,255);
		if(Head == "AmiigoUI_DrawFooter_0") SDL_SetRenderDrawColor(renderer,0 ,255 ,0 ,255);
		if(Head == "AmiigoUI_DrawFooter_1") SDL_SetRenderDrawColor(renderer,255 ,255 ,0 ,255);
		if(Head == "AmiigoUI_DrawFooter_2") SDL_SetRenderDrawColor(renderer,255 ,0 ,0 ,255);
		if(Head == "AmiigoUI_DrawFooter_3") SDL_SetRenderDrawColor(renderer,255 ,255 ,0 ,255);
		if(Head == "AmiigoUI_DrawFooter_D") SDL_SetRenderDrawColor(renderer,255 ,0 ,0 ,255);
		if(Head == "CreatorUI_DrawHeader") SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
		if(Head == "CreatorUI_DrawFooter_Select") SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		if(Head == "CreatorUI_DrawFooter_Back") SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		if(Head == "CreatorUI_PleaseWait") SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
		if(Head == "CreatorUI_DrawUI") SDL_SetRenderDrawColor(renderer, 94, 94, 94, 255);
		if(Head == "UpdaterUI_DrawText") SDL_SetRenderDrawColor(renderer, 0, 188, 212, 255);
		if (IDContents.size() != 0) IDContents = "";//reset json var
	}
}