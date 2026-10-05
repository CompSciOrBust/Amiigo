#pragma once

#include <arribaElements.h>
#include <elements/colourSlider.h>
#include <AmiigoSettings.h>
#include <functional>

namespace Amiigo::Elements {
    class ColourPickerDialog : public Arriba::Primitives::Quad {
    private:
        static const int NUM_FOCUS = 5;
        ColourSlider* sliders[4];
        Arriba::Primitives::Quad* preview;
        Arriba::Elements::Button* closeBtn;
        Arriba::UIObject* returnFocusTarget;
        colour* target;
        std::function<void()> onChange;
        std::function<void()> onClose;
        Arriba::UIObject* focusItems[NUM_FOCUS];
        bool firstFrame = true;
        bool initialHighlightSet = false;

        void updatePreview();
        void closeDialog();

    public:
        static const int DIALOG_W = 750;
        static const int DIALOG_H = 490;

        ColourPickerDialog(int x, int y, colour* target, const char32_t* colourName, std::function<void()> onChange, std::function<void()> onClose = nullptr);
        void onFrame() override;
    };
}  // namespace Amiigo::Elements
