#pragma once
#include <arribaElements.h>
#include <utils.h>
#include <vector>

namespace Amiigo::Elements {
    // Dirty hack to allow the context menu to delete things. How do we fix it properly? Don't ask me I'm offline in bulgaria
    inline AmiiboEntry amiiboEntryGlobal;
    class selectorContextMenu : public Arriba::Primitives::Quad {
        private:
            std::vector<Arriba::UIObject*> buttonVector;
            
        public:
            selectorContextMenu(int x, int y, AmiiboEntry entry);
            virtual void onFrame();
            void closeMenu();
    };
}  // namespace Amiigo::Elements
