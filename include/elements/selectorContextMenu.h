#pragma once

#include <arribaElements.h>
#include <utils.h>
#include <vector>

namespace Amiigo::Elements {
    inline AmiiboEntry amiiboEntryGlobal;
    class selectorContextMenu : public Arriba::Primitives::Quad {
        private:
            std::vector<Arriba::Elements::Button*> buttonVector;

        public:
            selectorContextMenu(int x, int y, AmiiboEntry entry);
            virtual void onFrame();
            void closeMenu();
    };
}  // namespace Amiigo::Elements
