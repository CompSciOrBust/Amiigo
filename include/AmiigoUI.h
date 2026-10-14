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

    void applySettingsQuadStyle(Arriba::Primitives::Quad* quad);
    void initUI();
    void initSplash();
    void initSceneSwitcher();
    void handleInput();
    void switcherPressed();
    enum class StatusLevel { Info, Error, Silent };
    void updateStatus(const char32_t* text, StatusLevel level = StatusLevel::Info);
}  // namespace Amiigo::UI
