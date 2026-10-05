#pragma once

#include <arribaElements.h>
#include <functional>
#include <vector>

namespace Amiigo::Elements {
    class CheckBox : public Arriba::Primitives::Quad {
    private:
        Arriba::Primitives::Quad* checkQuad;
        Arriba::Primitives::Quad* checkFill;
        Arriba::Primitives::Text* labelText;
        bool checkedState;
        std::vector<std::function<void(bool)>> callbacks;

    public:
        CheckBox(bool initialState, const char32_t* label);
        CheckBox(bool initialState, const char* label);
        void onFrame() override;
        void setChecked(bool state);
        bool isChecked() const;
        void registerCallback(std::function<void(bool)> cb);
    };
}  // namespace Amiigo::Elements
