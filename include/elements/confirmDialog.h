#pragma once

#include <arribaElements.h>
#include <functional>

namespace Amiigo::Elements {
    class ConfirmDialog : public Arriba::Primitives::Quad {
        Arriba::Elements::Button* yesBtn;
        Arriba::Elements::Button* noBtn;
        Arriba::UIObject* returnFocusTarget;
        std::function<void()> onConfirm;
        std::function<void()> onReject;
        bool firstFrame = true;
        bool initialHighlightSet = false;

        void close();

    public:
        static const int DIALOG_W = 620;
        static const int DIALOG_H = 210;

        ConfirmDialog(int x, int y, const char32_t* message, std::function<void()> onConfirm,
                      std::function<void()> onReject,
                      const char32_t* confirmLabel,
                      const char32_t* rejectLabel);
        void onFrame() override;
    };
}  // namespace Amiigo::Elements
