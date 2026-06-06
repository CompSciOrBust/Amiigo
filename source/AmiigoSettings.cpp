#include <AmiigoSettings.h>

#include <utils.h>

namespace {
    void loadColourFromTheme(const JsonDoc& j, const char* prefix, colour& c) {
        auto get = [&](const char* suffix, float& field) {
            std::string key = std::string(prefix) + suffix;
            if (j.contains(key)) field = j[key].get<float>();
        };
        get("_R", c.r); get("_G", c.g); get("_B", c.b); get("_A", c.a);
    }
}

namespace Amiigo::Settings {
    void loadSettings() {
        if (checkIfFileExists("sdmc:/config/amiigo/settings.json")) {
            JsonDoc settingsJson = loadJsonFile("sdmc:/config/amiigo/settings.json");
            int settingsVersion = 0;
            if (settingsJson.contains("settingsVersion")) settingsVersion = settingsJson["settingsVersion"].get<int>();
            if (settingsJson.contains("useRandomisedUUID")) useRandomisedUUID = settingsJson["useRandomisedUUID"].get<bool>();
            if (settingsJson.contains("timeToCheckUpdate")) updateTime = settingsJson["timeToCheckUpdate"].get<long unsigned int>();
            if (settingsJson.contains("categoryMode")) categoryMode = settingsJson["categoryMode"].get<unsigned char>() % categoryModes::categoryCount;
            if (settingsJson.contains("saveAmiiboImages")) saveAmiiboImages = settingsJson["saveAmiiboImages"].get<bool>();

            if (settingsVersion < 1) {
                saveAmiiboImages = true;
                saveSettings();
            }
        }

        if (checkIfFileExists("sdmc:/config/amiigo/theme.json")) {
            JsonDoc themeJson = loadJsonFile("sdmc:/config/amiigo/theme.json");
            loadColourFromTheme(themeJson, "StatusBar_Neutral",       Colour::statusBar);
            loadColourFromTheme(themeJson, "SelectorList_Neutral",    Colour::listNeutral);
            loadColourFromTheme(themeJson, "SelectorList_HighlightA", Colour::listHighlightA);
            loadColourFromTheme(themeJson, "SelectorList_HighlightB", Colour::listHighlightB);
            loadColourFromTheme(themeJson, "AmiigoStore_Neutral",     Colour::makerNeutral);
            loadColourFromTheme(themeJson, "AmiigoStore_HighlightA",  Colour::makerHighlightA);
            loadColourFromTheme(themeJson, "AmiigoStore_HighlightB",  Colour::makerHighlightB);
            loadColourFromTheme(themeJson, "Settings_Neutral",        Colour::settingsNeutral);
            loadColourFromTheme(themeJson, "Settings_HighlightA",     Colour::settingsHighlightA);
            loadColourFromTheme(themeJson, "Settings_HighlightB",     Colour::settingsHighlightB);
        }
    }

    void saveSettings() {
        JsonDoc settingsJson;
        settingsJson["settingsVersion"] = 1;
        settingsJson["timeToCheckUpdate"] = updateTime;
        settingsJson["categoryMode"] = categoryMode;
        settingsJson["useRandomisedUUID"] = useRandomisedUUID;
        settingsJson["saveAmiiboImages"] = saveAmiiboImages;
        writeJsonFile("sdmc:/config/amiigo/settings.json", settingsJson);
    }
}  // namespace Amiigo::Settings
