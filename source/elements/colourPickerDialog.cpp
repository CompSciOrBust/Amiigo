#include <elements/colourPickerDialog.h>
#include <AmiigoLang.h>
#include <arribaText.h>
#include <cmath>

namespace Amiigo::Elements {
    ColourPickerDialog::ColourPickerDialog(int x, int y, colour* target, const char32_t* colourName, std::function<void()> onChange, std::function<void()> onClose)
        : Arriba::Primitives::Quad(x, y, DIALOG_W, DIALOG_H, Arriba::Graphics::Pivot::topLeft),
          target(target), onChange(onChange), onClose(onClose) {
        returnFocusTarget = Arriba::highlightedObject;
        setName("ColourPickerDialog");
        setColour({0.08f, 0.08f, 0.08f, 0.97f});
        Arriba::activeLayer++;

        Arriba::Primitives::Text* titleText = new Arriba::Primitives::Text(colourName, 38);
        titleText->setParent(this);
        titleText->transform.position = {this->width / 2, 30, 0};
        titleText->setColour({0.9f, 0.9f, 0.9f, 1});

        preview = new Arriba::Primitives::Quad(0, 0, 40, 40, Arriba::Graphics::Pivot::topRight);
        preview->setParent(this);
        preview->transform.position = {this->width - 10, 10};
        updatePreview();

        const Arriba::Maths::vec4<float> fillColours[4] = {
            {0.95f, 0.3f,  0.3f,  1},
            {0.3f,  0.95f, 0.3f,  1},
            {0.3f,  0.5f,  1.0f,  1},
            {0.85f, 0.85f, 0.85f, 1},
        };
        const char* labels[4] = {"R", "G", "B", "A"};
        float* channels[4] = {&target->r, &target->g, &target->b, &target->a};
        const int sliderY[4] = {65, 145, 225, 305};

        for (int i = 0; i < 4; i++) {
            sliders[i] = new ColourSlider(labels[i], *channels[i], fillColours[i]);
            sliders[i]->setParent(this);
            sliders[i]->transform.position = {15, (float)sliderY[i], 0};
            float* ch = channels[i];
            sliders[i]->registerCallback([this, ch](float v) {
                *ch = v;
                updatePreview();
                if (this->onChange) this->onChange();
            });
            focusItems[i] = sliders[i];
        }

        closeBtn = new Arriba::Elements::Button();
        closeBtn->setParent(this);
        closeBtn->setDimensions(380, 55, Arriba::Graphics::Pivot::centre);
        closeBtn->transform.position = {(float)DIALOG_W / 2, 455, 0};
        closeBtn->setText(Amiigo::Lang::get("settings_theme_close").c_str());
        closeBtn->registerCallback([this]() { closeDialog(); });
        focusItems[4] = closeBtn;
    }

    void ColourPickerDialog::updatePreview() {
        if (preview) preview->setColour(*target);
    }

    void ColourPickerDialog::closeDialog() {
        Arriba::activeLayer--;
        Arriba::UIObject* parent = returnFocusTarget ? returnFocusTarget->getParent() : nullptr;
        Arriba::highlightedObject = parent ? parent : returnFocusTarget;
        if (onClose) onClose();
        destroy();
    }

    void ColourPickerDialog::onFrame() {
        if (firstFrame) { firstFrame = false; return; }

        if (!initialHighlightSet) {
            Arriba::highlightedObject = focusItems[0];
            initialHighlightSet = true;
        }

        if (Arriba::Input::buttonDown(Arriba::Input::backButton)) { closeDialog(); return; }

        for (int i = 0; i < NUM_FOCUS; i++) {
            if (Arriba::highlightedObject != focusItems[i]) continue;
            if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) && i < NUM_FOCUS - 1) Arriba::highlightedObject = focusItems[i + 1];
            if (Arriba::Input::buttonDown(Arriba::Input::DPadUp) && i > 0) Arriba::highlightedObject = focusItems[i - 1];
            break;
        }
    }
}  // namespace Amiigo::Elements
