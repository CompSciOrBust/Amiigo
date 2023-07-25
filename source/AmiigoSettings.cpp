#include <AmiigoSettings.h>
#include <nlohmann/json.hpp>
#include <utils.h>
#include <string>
#include <fstream>

namespace Amiigo::Settings
{
    void loadSettings()
    {
        // Parse main config
        if(checkIfFileExists("sdmc:/config/amiigo/settings.json")) {
            //Load the file from the sd
            std::ifstream fileStream("sdmc:/config/amiigo/settings.json");
            std::string tempLine;
            std::string fileString;
            while(getline(fileStream, tempLine)) fileString += tempLine;

            //Parse the file
            nlohmann::json settingsJson = nlohmann::json::parse(fileString);
            fileStream.close();
            if(settingsJson.contains("saveAmiibosToCategory")) saveAmiibosToCategory = settingsJson["saveAmiibosToCategory"].get<bool>();
            if(settingsJson.contains("useRandomisedUUID")) useRandomisedUUID = settingsJson["useRandomisedUUID"].get<bool>();
            if(settingsJson.contains("timeToCheckUpdate")) updateTime = settingsJson["timeToCheckUpdate"].get<long unsigned int>();
            if(settingsJson.contains("categoryMode")) categoryMode = settingsJson["categoryMode"].get<unsigned char>() % Amiigo::Settings::categoryModes::categoryCount;

            //Back compat with versions < 2.2.0
            if(!settingsJson.contains("categoryMode") && settingsJson.contains("saveAmiibosToCategory")) {
                if(saveAmiibosToCategory) Amiigo::Settings::categoryMode = Amiigo::Settings::saveByGameName;
                else Amiigo::Settings::categoryMode = Amiigo::Settings::saveToRoot;
                saveSettings();
            }
            if (!settingsJson.contains("useRandomisedUUID")) saveSettings();
        }

        // Parse theme config
        if(checkIfFileExists("sdmc:/config/amiigo/theme.json")) {
            // Load the theme file
            std::ifstream fileStream("sdmc:/config/amiigo/theme.json");
            std::string tempLine;
            std::string fileString;
            while(getline(fileStream, tempLine)) fileString += tempLine;

            // Parse the file to load the colours
            nlohmann::json themeJson = nlohmann::json::parse(fileString);
            fileStream.close();

            // Status bar
            if(themeJson.contains("StatusBar_Neutral_R")) Colour::statusBar.r = themeJson["StatusBar_Neutral_R"].get<float>();
            if(themeJson.contains("StatusBar_Neutral_G")) Colour::statusBar.g = themeJson["StatusBar_Neutral_G"].get<float>();
            if(themeJson.contains("StatusBar_Neutral_B")) Colour::statusBar.b = themeJson["StatusBar_Neutral_B"].get<float>();
            if(themeJson.contains("StatusBar_Neutral_A")) Colour::statusBar.a = themeJson["StatusBar_Neutral_A"].get<float>();

            // Amiibo list neutral
            if(themeJson.contains("SelectorList_Neutral_R")) Colour::listNeutral.r = themeJson["SelectorList_Neutral_R"].get<float>();
            if(themeJson.contains("SelectorList_Neutral_G")) Colour::listNeutral.g = themeJson["SelectorList_Neutral_G"].get<float>();
            if(themeJson.contains("SelectorList_Neutral_B")) Colour::listNeutral.b = themeJson["SelectorList_Neutral_B"].get<float>();
            if(themeJson.contains("SelectorList_Neutral_A")) Colour::listNeutral.a = themeJson["SelectorList_Neutral_A"].get<float>();
            // Amiibo list highlightA
            if(themeJson.contains("SelectorList_HighlightA_R")) Colour::listHighlightA.r = themeJson["SelectorList_HighlightA_R"].get<float>();
            if(themeJson.contains("SelectorList_HighlightA_G")) Colour::listHighlightA.g = themeJson["SelectorList_HighlightA_G"].get<float>();
            if(themeJson.contains("SelectorList_HighlightA_B")) Colour::listHighlightA.b = themeJson["SelectorList_HighlightA_B"].get<float>();
            if(themeJson.contains("SelectorList_HighlightA_A")) Colour::listHighlightA.a = themeJson["SelectorList_HighlightA_A"].get<float>();
            // Amiibo list highlightB
            if(themeJson.contains("SelectorList_HighlightB_R")) Colour::listHighlightB.r = themeJson["SelectorList_HighlightB_R"].get<float>();
            if(themeJson.contains("SelectorList_HighlightB_G")) Colour::listHighlightB.g = themeJson["SelectorList_HighlightB_G"].get<float>();
            if(themeJson.contains("SelectorList_HighlightB_B")) Colour::listHighlightB.b = themeJson["SelectorList_HighlightB_B"].get<float>();
            if(themeJson.contains("SelectorList_HighlightB_A")) Colour::listHighlightB.a = themeJson["SelectorList_HighlightB_A"].get<float>();

            // Amiigo store neutral
            if(themeJson.contains("AmiigoStore_Neutral_R")) Colour::makerNeutral.r = themeJson["AmiigoStore_Neutral_R"].get<float>();
            if(themeJson.contains("AmiigoStore_Neutral_G")) Colour::makerNeutral.g = themeJson["AmiigoStore_Neutral_G"].get<float>();
            if(themeJson.contains("AmiigoStore_Neutral_B")) Colour::makerNeutral.b = themeJson["AmiigoStore_Neutral_B"].get<float>();
            if(themeJson.contains("AmiigoStore_Neutral_A")) Colour::makerNeutral.a = themeJson["AmiigoStore_Neutral_A"].get<float>();
            // Amiigo store highlightA
            if(themeJson.contains("AmiigoStore_HighlightA_R")) Colour::makerHighlightA.r = themeJson["AmiigoStore_HighlightA_R"].get<float>();
            if(themeJson.contains("AmiigoStore_HighlightA_G")) Colour::makerHighlightA.g = themeJson["AmiigoStore_HighlightA_G"].get<float>();
            if(themeJson.contains("AmiigoStore_HighlightA_B")) Colour::makerHighlightA.b = themeJson["AmiigoStore_HighlightA_B"].get<float>();
            if(themeJson.contains("AmiigoStore_HighlightA_A")) Colour::makerHighlightA.a = themeJson["AmiigoStore_HighlightA_A"].get<float>();
            // Amiigo store highlightB
            if(themeJson.contains("AmiigoStore_HighlightB_R")) Colour::makerHighlightB.r = themeJson["AmiigoStore_HighlightB_R"].get<float>();
            if(themeJson.contains("AmiigoStore_HighlightB_G")) Colour::makerHighlightB.g = themeJson["AmiigoStore_HighlightB_G"].get<float>();
            if(themeJson.contains("AmiigoStore_HighlightB_B")) Colour::makerHighlightB.b = themeJson["AmiigoStore_HighlightB_B"].get<float>();
            if(themeJson.contains("AmiigoStore_HighlightB_A")) Colour::makerHighlightB.a = themeJson["AmiigoStore_HighlightB_A"].get<float>();

            // Amiigo settings neutral
            if(themeJson.contains("Settings_Neutral_R")) Colour::settingsNeutral.r = themeJson["Settings_Neutral_R"].get<float>();
            if(themeJson.contains("Settings_Neutral_G")) Colour::settingsNeutral.g = themeJson["Settings_Neutral_G"].get<float>();
            if(themeJson.contains("Settings_Neutral_B")) Colour::settingsNeutral.b = themeJson["Settings_Neutral_B"].get<float>();
            if(themeJson.contains("Settings_Neutral_A")) Colour::settingsNeutral.a = themeJson["Settings_Neutral_A"].get<float>();
            // Amiigo settings highlightA
            if(themeJson.contains("Settings_HighlightA_R")) Colour::settingsHighlightA.r = themeJson["Settings_HighlightA_R"].get<float>();
            if(themeJson.contains("Settings_HighlightA_G")) Colour::settingsHighlightA.g = themeJson["Settings_HighlightA_G"].get<float>();
            if(themeJson.contains("Settings_HighlightA_B")) Colour::settingsHighlightA.b = themeJson["Settings_HighlightA_B"].get<float>();
            if(themeJson.contains("Settings_HighlightA_A")) Colour::settingsHighlightA.a = themeJson["Settings_HighlightA_A"].get<float>();
            // Amiigo settings highlightB
            if(themeJson.contains("Settings_HighlightB_R")) Colour::settingsHighlightB.r = themeJson["Settings_HighlightB_R"].get<float>();
            if(themeJson.contains("Settings_HighlightB_G")) Colour::settingsHighlightB.g = themeJson["Settings_HighlightB_G"].get<float>();
            if(themeJson.contains("Settings_HighlightB_B")) Colour::settingsHighlightB.b = themeJson["Settings_HighlightB_B"].get<float>();
            if(themeJson.contains("Settings_HighlightB_A")) Colour::settingsHighlightB.a = themeJson["Settings_HighlightB_A"].get<float>();
        }
    }

    void saveSettings()
    {
        if(checkIfFileExists("sdmc:/config/amiigo/settings.json")) std::remove("sdmc:/config/amiigo/settings.json");
        std::ofstream fileStream("sdmc:/config/amiigo/settings.json");
        nlohmann::json settingsJson;
        settingsJson["timeToCheckUpdate"] = updateTime;
        settingsJson["categoryMode"] = categoryMode;
        settingsJson["useRandomisedUUID"] = useRandomisedUUID;
        fileStream << settingsJson << std::endl;
        fileStream.close();
    }
}