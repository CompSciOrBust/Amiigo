#include <AmiigoSettings.h>
#include <nlohmann/json.hpp>
#include <utils.h>
#include <string>
#include <fstream>

namespace Amiigo::Settings
{
    void loadSettings()
    {
        if(!checkIfFileExists("sdmc:/config/amiigo/settings.json")) return;
        //Load the file from the sd
        std::ifstream fileStream("sdmc:/config/amiigo/settings.json");
        std::string tempLine;
        std::string fileString;
        while(getline(fileStream, tempLine)) fileString += tempLine;
        //Parse the file
        nlohmann::json settingsJson = nlohmann::json::parse(fileString);
        fileStream.close();
        if(settingsJson.contains("saveAmiibosToCategory")) saveAmiibosToCategory = settingsJson["saveAmiibosToCategory"].get<bool>();
        if(settingsJson.contains("timeToCheckUpdate")) updateTime = settingsJson["timeToCheckUpdate"].get<long unsigned int>();
        if(settingsJson.contains("categoryMode")) categoryMode = settingsJson["categoryMode"].get<unsigned char>() % Amiigo::Settings::categoryModes::categoryCount;
        //Back compat with versions < 2.2.0
        if(!settingsJson.contains("categoryMode") && settingsJson.contains("saveAmiibosToCategory"))
        {
            if(saveAmiibosToCategory) Amiigo::Settings::categoryMode = Amiigo::Settings::saveByGameName;
            else Amiigo::Settings::categoryMode = Amiigo::Settings::saveToRoot;
            saveSettings();
        }
    }

    void saveSettings()
    {
        if(checkIfFileExists("sdmc:/config/amiigo/settings.json")) std::remove("sdmc:/config/amiigo/settings.json");
        std::ofstream fileStream("sdmc:/config/amiigo/settings.json");
        nlohmann::json settingsJson;
        settingsJson["timeToCheckUpdate"] = updateTime;
        settingsJson["categoryMode"] = categoryMode;
        fileStream << settingsJson << std::endl;
        fileStream.close();
    }
}