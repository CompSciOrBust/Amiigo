#pragma once

#include <arribaElements.h>
#include <functional>
#include <string>
#include <vector>

namespace Amiigo::Elements {
    class DropdownMenu : public Arriba::Primitives::Quad {
        private:
            std::vector<Arriba::Elements::Button*> buttonVector;
            Arriba::UIObject* returnFocusTarget;
            bool firstFrame = true;
            bool initialHighlightSet = false;
            int pendingIndex = 0;

        public:
            struct Option {
                std::u32string label;
                std::function<void()> callback;
            };

            DropdownMenu(int x, int y, int width, const std::vector<Option>& options, int selectedIndex = 0);
            void onFrame() override;
            void closeMenu();
    };
}  // namespace Amiigo::Elements
