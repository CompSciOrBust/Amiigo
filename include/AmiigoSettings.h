#pragma once

#include <arribaMaths.h>
#include <switch.h>
#include <string>

namespace Amiigo::Settings {
    inline bool useRandomisedUUID = false;
    inline char amiigoPath[FS_MAX_PATH];
    inline long unsigned int updateTime = 0;
    inline unsigned char categoryMode = 0;
    inline bool saveAmiiboImages = true;
    void loadSettings();
    void saveSettings();
    void saveTheme();

    enum categoryModes  {
        saveToRoot,
        saveByGameName,
        saveByAmiiboSeries,
        saveByCurrentFolder,
        categoryCount = saveByCurrentFolder+1
    };
}  // namespace Amiigo::Settings

typedef Arriba::Maths::vec4<float> colour;

namespace Amiigo::Settings::Colour {
    namespace Defaults {
        inline constexpr colour statusBar          = {0.5f,  0.7f,  0.7f,  0.9f};
        inline constexpr colour listNeutral        = {0.22f, 0.47f, 0.93f, 0.97f};
        inline constexpr colour listHighlightA     = {0.1f,  0.95f, 0.98f, 0.97f};
        inline constexpr colour listHighlightB     = {0.5f,  0.85f, 1.0f,  0.97f};
        inline constexpr colour makerNeutral       = {0.20f, 0.76f, 0.45f, 0.97f};
        inline constexpr colour makerHighlightA    = {0.6f,  0.95f, 0.98f, 0.97f};
        inline constexpr colour makerHighlightB    = {0.1f,  0.98f, 0.55f, 0.97f};
        inline constexpr colour settingsNeutral    = {0.57f, 0.21f, 0.93f, 0.97f};
        inline constexpr colour settingsHighlightA = {0.9f,  0.95f, 0.94f, 0.97f};
        inline constexpr colour settingsHighlightB = {1.0f,  0.85f, 1.0f,  0.97f};
    }  // namespace Defaults
    inline colour statusBar          = Defaults::statusBar;
    inline colour listNeutral        = Defaults::listNeutral;
    inline colour listHighlightA     = Defaults::listHighlightA;
    inline colour listHighlightB     = Defaults::listHighlightB;
    inline colour makerNeutral       = Defaults::makerNeutral;
    inline colour makerHighlightA    = Defaults::makerHighlightA;
    inline colour makerHighlightB    = Defaults::makerHighlightB;
    inline colour settingsNeutral    = Defaults::settingsNeutral;
    inline colour settingsHighlightA = Defaults::settingsHighlightA;
    inline colour settingsHighlightB = Defaults::settingsHighlightB;
}  // namespace Amiigo::Settings::Colour
