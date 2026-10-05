#include <elements/checkBox.h>
#include <arribaText.h>

namespace Amiigo::Elements {
    CheckBox::CheckBox(bool initialState, const char32_t* label)
        : Arriba::Primitives::Quad(0, 0, 700, 100, Arriba::Graphics::Pivot::topLeft) {
        checkedState = initialState;
        setColour({0, 0, 0, 0});

        checkQuad = new Arriba::Primitives::Quad(0, 0, 60, 60, Arriba::Graphics::Pivot::topLeft);
        checkQuad->setParent(this);
        checkQuad->transform.position = {0, this->height / 2 - checkQuad->height / 2, 0};
        checkQuad->setColour(Arriba::Colour::neutral);

        checkFill = new Arriba::Primitives::Quad(0, 0, 48, 48, Arriba::Graphics::Pivot::centre);
        checkFill->setParent(checkQuad);
        checkFill->transform.position = {checkQuad->width / 2, checkQuad->width / 2, 0};
        checkFill->setColour({0, 0, 0, 1});
        checkFill->enabled = initialState;

        labelText = new Arriba::Primitives::Text(label, 48);
        labelText->setParent(checkQuad);
        labelText->transform.position = {checkQuad->width + labelText->width / 2 + 15, checkQuad->height / 2, 0};
        labelText->setColour({1, 1, 1, 1});

        this->width = labelText->getRight() - checkQuad->getLeft();
    }

    CheckBox::CheckBox(bool initialState, const char* label) : CheckBox(initialState, Arriba::Text::ASCIIToUnicode(label).c_str()) {}

    void CheckBox::onFrame() {
        bool activated = false;
        if (Arriba::Input::buttonDown(Arriba::Input::confirmButton) && Arriba::highlightedObject == this) activated = true;

        float touchX = Arriba::Input::touch.pos.x;
        float touchY = Arriba::Input::touch.pos.y;
        bool isTouched = false;
        if (Arriba::Input::touchScreenPressed() && Arriba::activeLayer == layer) {
            bool withinBounds = touchY < getTop() && touchY > getBottom() && touchX < getRight() && touchX > getLeft();
            if (Arriba::Input::touch.start) {
                if (withinBounds) Arriba::highlightedObject = this;
                else if (Arriba::highlightedObject == this) Arriba::highlightedObject = nullptr;
            }
            if (Arriba::highlightedObject == this) isTouched = withinBounds;
        }
        if (Arriba::highlightedObject == this && Arriba::Input::touch.end && Arriba::activeLayer == layer) {
            if (touchY < getTop() && touchY > getBottom() && touchX < getRight() && touchX > getLeft()) activated = true;
        }

        if (activated) {
            checkedState = !checkedState;
            checkFill->enabled = checkedState;
            for (auto& cb : callbacks) cb(checkedState);
        }

        Arriba::Maths::vec4<float> targetColour = Arriba::Colour::neutral;
        float lerpValue = (sin(Arriba::time * 4) + 1) / 2;
        if (Arriba::highlightedObject == this) {
            targetColour = Arriba::Maths::lerp(Arriba::Colour::highlightA, Arriba::Colour::highlightB, lerpValue);
            if (activated || isTouched) checkQuad->setColour(Arriba::Colour::activatedColour);
        }
        checkQuad->setColour(Arriba::Maths::lerp(checkQuad->getColour(), targetColour, 3 * Arriba::deltaTime));
    }

    void CheckBox::setChecked(bool state) {
        checkedState = state;
        checkFill->enabled = state;
    }

    bool CheckBox::isChecked() const { return checkedState; }

    void CheckBox::registerCallback(std::function<void(bool)> cb) { callbacks.push_back(cb); }
}  // namespace Amiigo::Elements
