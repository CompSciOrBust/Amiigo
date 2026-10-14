#pragma once
#include <arriba.h>

namespace Amiigo::UI {
    void initMaker();
    void handleMakerInput();
    void makerInput(int index);
    void makerContextMenuSpawner(int index, Arriba::Maths::vec2<float> pos);
    void resetMakerToRoot();
}
