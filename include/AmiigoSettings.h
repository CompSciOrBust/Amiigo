#pragma once
#include <string>
#include <switch.h>

namespace Amiigo::Settings
{
    //Vars
    inline bool saveAmiibosToCategory = false;
    inline bool useRandomisedUUID = false;
    inline std::string updateURL = "";
    inline char amiigoPath[FS_MAX_PATH];
    inline long unsigned int updateTime = 0;
    inline unsigned char categoryMode = 0;
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
