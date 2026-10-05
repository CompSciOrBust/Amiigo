#pragma once

#include <arribaElements.h>
#include <functional>
#include <vector>

namespace Amiigo::Elements {
    class ColourSlider : public Arriba::Primitives::Quad {
    private:
        Arriba::Primitives::Quad* track;
        Arriba::Primitives::Text* valueText;
        float currentValue;
        bool touching = false;
        std::vector<std::function<void(float)>> callbacks;

        void applyValue(float v);

    public:
        static const int SLIDER_WIDTH  = 700;
        static const int SLIDER_HEIGHT = 72;
        static const int TRACK_X       = 42;
        static const int TRACK_W       = 590;
        static const int TRACK_H       = 28;

        ColourSlider(const char* label, float initialValue, Arriba::Maths::vec4<float> fillColour);
        void onFrame() override;
        float getValue() const;
        void setValue(float v);
        void registerCallback(std::function<void(float)> cb);
    };
}  // namespace Amiigo::Elements
