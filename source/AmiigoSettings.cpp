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
    }

    void saveSettings()
    {
        if(checkIfFileExists("sdmc:/config/amiigo/settings.json")) std::remove("sdmc:/config/amiigo/settings.json");
        std::ofstream fileStream("sdmc:/config/amiigo/settings.json");
        nlohmann::json settingsJson;
        settingsJson["saveAmiibosToCategory"] = saveAmiibosToCategory;
        settingsJson["timeToCheckUpdate"] = updateTime;
        fileStream << settingsJson << std::endl;
        fileStream.close();
    }
}