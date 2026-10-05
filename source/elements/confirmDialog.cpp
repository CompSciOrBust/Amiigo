#include <elements/confirmDialog.h>
#include <AmiigoLang.h>
#include <AmiigoUI.h>

namespace Amiigo::Elements {
    ConfirmDialog::ConfirmDialog(int x, int y, const char32_t* message, std::function<void()> onConfirm)
        : Arriba::Primitives::Quad(x, y, DIALOG_W, DIALOG_H, Arriba::Graphics::Pivot::topLeft),
          onConfirm(onConfirm) {
        returnFocusTarget = Arriba::highlightedObject;
        setName("ConfirmDialog");

        setColour({0.08f, 0.08f, 0.08f, 0.97f});
        renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "romfs:/dialogFragment.glsl");
        renderer->thisShader.setFloat1("aspectRatio", (float)DIALOG_W / DIALOG_H);
        renderer->thisShader.setFloat1("radius", 30.0f / DIALOG_H);
        renderer->thisShader.setFloat1("outlineWidth", 2.0f / DIALOG_H);
        Arriba::activeLayer++;

        Arriba::Primitives::Text* msgText = new Arriba::Primitives::Text(message, 38);
        msgText->setParent(this);
        msgText->transform.position = {(float)DIALOG_W / 2, 80, 0};
        msgText->setColour({0.9f, 0.9f, 0.9f, 1});

        noBtn = new Arriba::Elements::Button();
        noBtn->setParent(this);
        noBtn->setDimensions(200, 70, Arriba::Graphics::Pivot::centre);
        noBtn->transform.position = {(float)DIALOG_W / 2 - 130, 160, 0};
        noBtn->setText(Amiigo::Lang::get("settings_generate_all_no").c_str());
        noBtn->registerCallback([this]() { close(); });
        Amiigo::UI::applySettingsQuadStyle(noBtn);

        yesBtn = new Arriba::Elements::Button();
        yesBtn->setParent(this);
        yesBtn->setDimensions(200, 70, Arriba::Graphics::Pivot::centre);
        yesBtn->transform.position = {(float)DIALOG_W / 2 + 130, 160, 0};
        yesBtn->setText(Amiigo::Lang::get("settings_generate_all_yes").c_str());
        yesBtn->registerCallback([this]() {
            if (this->onConfirm) this->onConfirm();
            close();
        });
        Amiigo::UI::applySettingsQuadStyle(yesBtn);
    }

    void ConfirmDialog::close() {
        Arriba::activeLayer--;
        Arriba::UIObject* parent = returnFocusTarget ? returnFocusTarget->getParent() : nullptr;
        Arriba::highlightedObject = parent ? parent : returnFocusTarget;
        destroy();
    }

    void ConfirmDialog::onFrame() {
        if (firstFrame) {
            firstFrame = false;
            return;
        }

        if (!initialHighlightSet) {
            Arriba::highlightedObject = noBtn;
            initialHighlightSet = true;
        }

        if (Arriba::Input::buttonDown(Arriba::Input::backButton)) { close(); return; }
        if (Arriba::Input::buttonDown(Arriba::Input::DPadLeft) && Arriba::highlightedObject == yesBtn) Arriba::highlightedObject = noBtn;
        if (Arriba::Input::buttonDown(Arriba::Input::DPadRight) && Arriba::highlightedObject == noBtn) Arriba::highlightedObject = yesBtn;
    }
}  // namespace Amiigo::Elements
