#pragma once

#include <arribaElements.h>
#include <atomic>
#include <memory>

namespace Amiigo::Elements {
    class ProgressDialog : public Arriba::Primitives::Quad {
        std::shared_ptr<std::atomic<int>> progress;
        int total;
        Arriba::Primitives::Quad* progressBar;
        Arriba::Primitives::Text* progressText;
        Arriba::Elements::Button* doneBtn;
        bool doneFocusSet = false;
        bool firstFrame = true;

        void closeDialog();

    public:
        static const int DIALOG_W = 700;
        static const int DIALOG_H = 265;

        ProgressDialog(int x, int y, const char32_t* title, std::shared_ptr<std::atomic<int>> progress, int total);
        void onFrame() override;
    };
}  // namespace Amiigo::Elements
