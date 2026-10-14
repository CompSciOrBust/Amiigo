#pragma once
#include <arriba.h>
#include <string>

namespace Amiigo::UI {
    void initSelector();
    void handleSelectorInput();
    void selectorInput(int index);
    void selectorContextMenuSpawner(int index, Arriba::Maths::vec2<float> pos);
    void updateSelectorStrings();
    void resetSelectorToRoot();
    const std::string& getSelectorPath();
}
