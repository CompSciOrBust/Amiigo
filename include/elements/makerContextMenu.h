#pragma once

#include <arribaElements.h>
#include <utils.h>
#include <vector>

namespace Amiigo::Elements {
    class MakerContextMenu : public Arriba::Primitives::Quad {
        private:
            std::vector<Arriba::Elements::Button*> buttonVector;

        public:
            MakerContextMenu(int x, int y, const std::string& series);
            virtual void onFrame();
            void closeMenu();
    };
}  // namespace Amiigo::Elements
