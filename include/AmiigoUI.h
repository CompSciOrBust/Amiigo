#pragma once
#include <arriba.h>
#include <arribaElements.h>
#include <arribaPrimitives.h>
#include <utils.h>
#include <vector>
#include <string>

namespace Amiigo::UI {
    // Variables
    inline int isRunning = 1;
    inline int statusHeight = Arriba::Graphics::windowHeight * 0.1;
    inline int switcherWidth = Arriba::Graphics::windowWidth * 0.3;
    inline int switcherHeight = Arriba::Graphics::windowHeight - statusHeight;
    inline std::vector<std::string> seriesList;
    inline std::vector<AmiiboCreatorData> creatorData;
    inline bool makerIsInCategory = false;
    inline std::string selectedSeries;
    inline std::vector<AmiiboEntry> selectorAmiibos;
    inline std::string selectorPath = "sdmc:/emuiibo/amiibo";
    // UI Object pointers
    // Scene bases
    inline Arriba::Primitives::Quad* splashScene = nullptr;
    inline Arriba::Primitives::Quad* selectorScene = nullptr;
    inline Arriba::Primitives::Quad* sceneSwitcher = nullptr;
    inline Arriba::Primitives::Quad* makerSwitcher = nullptr;
    // Lists
    inline std::vector<Arriba::UIObject*> lists;
    inline Arriba::Elements::InertialList* selectorList = nullptr;
    inline Arriba::Elements::InertialList* makerList = nullptr;
    // Switcher buttons
    inline std::vector<Arriba::UIObject*> buttons;
    inline Arriba::Elements::Button* selectorButton = nullptr;
    inline Arriba::Elements::Button* makerButton = nullptr;
    inline Arriba::Elements::Button* settingsButton = nullptr;
    inline Arriba::Elements::Button* exitButton = nullptr;
    // Settings objects
    inline Arriba::Primitives::Quad* settingsScene = nullptr;


    void initUI();
    void initSplash();
    void initSceneSwitcher();
    void initSelector();
    void initMaker();
    void initSettings();
    void handleInput();
    void switcherPressed();
    void selectorInput(int index);
    void makerInput(int index);
    void updateSelectorStrings();
    void selectorContextMenuSpawner(int index, Arriba::Maths::vec2<float> pos);
    void updateStatusInfo(const char32_t* text);
    void updateStatusError(const char32_t* text);
    void updateStatusSilent(const char32_t* text);
}  // namespace Amiigo::UI
