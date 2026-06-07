#pragma once

#include <arribaElements.h>
#include <utils.h>
#include <vector>

namespace Amiigo::Elements {
    class SelectorContextMenu : public Arriba::Primitives::Quad {
        private:
            std::vector<Arriba::Elements::Button*> buttonVector;

        public:
            SelectorContextMenu(int x, int y, AmiiboEntry entry);
            virtual void onFrame();
            void closeMenu();
    };
}  // namespace Amiigo::Elements
