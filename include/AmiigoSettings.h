#pragma once
#include <string>
#include <switch.h>
#include <arribaMaths.h>

namespace Amiigo::Settings
{
    //Vars
    inline bool saveAmiibosToCategory = false;
    inline bool useRandomisedUUID = false;
    inline std::string updateURL = "";
    inline char amiigoPath[FS_MAX_PATH];
    inline long unsigned int updateTime = 0;
    inline unsigned char categoryMode = 0;
    inline std::string emuiiboVersionText = "";
    void loadSettings();
    void saveSettings();

    enum categoryModes  {
        saveToRoot,
        saveByGameName,
        saveByAmiiboSeries,
        saveByCurrentFolder,
        categoryCount = saveByCurrentFolder+1
    };
}

typedef Arriba::Maths::vec4<float> colour;

namespace Amiigo::Settings::Colour {
    // Header
    inline colour statusBar = {0.5,0.7,0.7,0.9};
    // Amiibo list
    inline colour listNeutral = {0.22,0.47,0.93,0.97};
    inline colour listHighlightA = {0.1,0.95,0.98,0.97};
    inline colour listHighlightB = {0.5,0.85,1,0.97};
    // Amiigo Maker
    inline colour makerNeutral = {0.20,0.76,0.45,0.97};
    inline colour makerHighlightA = {0.6,0.95,0.98,0.97};
    inline colour makerHighlightB = {0.1,0.98,0.55,0.97};
    // Settings
    inline colour settingsNeutral = {0.57,0.21,0.93,0.97};
    inline colour settingsHighlightA = {0.9,0.95,0.94,0.97};
    inline colour settingsHighlightB = {1,0.85,1,0.97};
}