#include <elements/colourSlider.h>
#include <arribaText.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

namespace Amiigo::Elements {
    ColourSlider::ColourSlider(const char* label, float initialValue, Arriba::Maths::vec4<float> fillColour)
        : Arriba::Primitives::Quad(0, 0, SLIDER_WIDTH, SLIDER_HEIGHT, Arriba::Graphics::Pivot::topLeft),
          currentValue(std::max(0.0f, std::min(1.0f, initialValue))) {
        setColour({0, 0, 0, 0});

        Arriba::Primitives::Text* labelText = new Arriba::Primitives::Text(label, 40);
        labelText->setParent(this);
        labelText->transform.position = {20, (float)SLIDER_HEIGHT / 2, 0};
        labelText->setColour({1, 1, 1, 1});

        track = new Arriba::Primitives::Quad(TRACK_X, (SLIDER_HEIGHT - TRACK_H) / 2, TRACK_W, TRACK_H, Arriba::Graphics::Pivot::topLeft);
        track->setParent(this);
        track->setColour({0.2f, 0.2f, 0.2f, 1.0f});

        fill = new Arriba::Primitives::Quad(0, 0, (int)(currentValue * TRACK_W), TRACK_H, Arriba::Graphics::Pivot::topLeft);
        fill->setParent(track);
        fill->setColour(fillColour);

        char buf[8];
        snprintf(buf, sizeof(buf), "%.2f", currentValue);
        valueText = new Arriba::Primitives::Text(buf, 36);
        valueText->setParent(track);
        valueText->transform.position = {track->width + valueText->width / 2 + 15, track->height / 2, 0};
        valueText->setColour({1, 1, 1, 1});
    }

    void ColourSlider::applyValue(float v) {
        currentValue = std::max(0.0f, std::min(1.0f, v));
        fill->setDimensions((int)(currentValue * TRACK_W), TRACK_H, Arriba::Graphics::Pivot::topLeft);
        char buf[8];
        snprintf(buf, sizeof(buf), "%.2f", currentValue);
        valueText->setText(buf);
        for (auto& cb : callbacks) cb(currentValue);
    }

    void ColourSlider::onFrame() {
        bool isHighlighted = Arriba::highlightedObject == this;

        if (isHighlighted && Arriba::activeLayer == layer) {
            if (Arriba::Input::buttonDown(Arriba::Input::DPadLeft))  applyValue(currentValue - 0.01f);
            if (Arriba::Input::buttonDown(Arriba::Input::DPadRight)) applyValue(currentValue + 0.01f);

            float stickX = Arriba::Input::AnalogStickLeft.xPos + Arriba::Input::AnalogStickRight.xPos;
            if (std::abs(stickX) > 0.1f) applyValue(currentValue + stickX * (float)Arriba::deltaTime);
        }

        float touchX = Arriba::Input::touch.pos.x;
        float touchY = Arriba::Input::touch.pos.y;
        if (Arriba::Input::touchScreenPressed() && Arriba::activeLayer == layer) {
            bool within = touchY < getTop() && touchY > getBottom() && touchX < getRight() && touchX > getLeft();
            if (Arriba::Input::touch.start) {
                if (within) { Arriba::highlightedObject = this; touching = true; }
                else if (isHighlighted) { Arriba::highlightedObject = nullptr; touching = false; }
            }
            if (touching) {
                float trackStart = getLeft() + TRACK_X;
                applyValue((touchX - trackStart) / TRACK_W);
            }
        }
        if (Arriba::Input::touch.end) touching = false;

        Arriba::Maths::vec4<float> targetColour = {0, 0, 0, 0};
        if (isHighlighted) {
            float lerpValue = (std::sin((float)Arriba::time * 4) + 1) / 2;
            auto highlight = Arriba::Maths::lerp(Arriba::Colour::highlightA, Arriba::Colour::highlightB, lerpValue);
            targetColour = {highlight.r, highlight.g, highlight.b, 0.3f};
        }
        setColour(Arriba::Maths::lerp(getColour(), targetColour, (float)(3 * Arriba::deltaTime)));
    }

    float ColourSlider::getValue() const { return currentValue; }

    void ColourSlider::setValue(float v) {
        currentValue = std::max(0.0f, std::min(1.0f, v));
        fill->setDimensions((int)(currentValue * TRACK_W), TRACK_H, Arriba::Graphics::Pivot::topLeft);
        char buf[8];
        snprintf(buf, sizeof(buf), "%.2f", currentValue);
        valueText->setText(buf);
    }

    void ColourSlider::registerCallback(std::function<void(float)> cb) { callbacks.push_back(cb); }
}  // namespace Amiigo::Elements
