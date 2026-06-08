#pragma once

#include <arriba.h>
#include <arribaElements.h>
#include <arribaPrimitives.h>
#include <utils.h>
#include <vector>
#include <string>

namespace Amiigo::UI {
    inline int isRunning = 1;
    inline const int statusHeight = Arriba::Graphics::windowHeight * 0.1;
    inline const int switcherWidth = Arriba::Graphics::windowWidth * 0.3;
    inline const int switcherHeight = Arriba::Graphics::windowHeight - statusHeight;

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
    enum class StatusLevel { Info, Error, Silent };
    void updateStatus(const char32_t* text, StatusLevel level = StatusLevel::Info);
    const std::string& getSelectorPath();
}  // namespace Amiigo::UI
