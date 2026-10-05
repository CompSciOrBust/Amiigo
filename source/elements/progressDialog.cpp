#include <elements/progressDialog.h>
#include <AmiigoLang.h>
#include <AmiigoUI.h>
#include <cstdio>

namespace Amiigo::Elements {
    ProgressDialog::ProgressDialog(int x, int y, const char32_t* title, std::shared_ptr<std::atomic<int>> progress, int total)
        : Arriba::Primitives::Quad(x, y, DIALOG_W, DIALOG_H, Arriba::Graphics::Pivot::topLeft),
          progress(progress), total(total) {
        setName("ProgressDialog");
        setColour({0.08f, 0.08f, 0.08f, 0.97f});
        renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "romfs:/dialogFragment.glsl");
        renderer->thisShader.setFloat1("aspectRatio", (float)DIALOG_W / DIALOG_H);
        renderer->thisShader.setFloat1("radius", 30.0f / DIALOG_H);
        renderer->thisShader.setFloat1("outlineWidth", 2.0f / DIALOG_H);
        Arriba::activeLayer++;

        Arriba::Primitives::Text* titleText = new Arriba::Primitives::Text(title, 38);
        titleText->setParent(this);
        titleText->transform.position = {(float)DIALOG_W / 2, 40, 0};
        titleText->setColour({0.9f, 0.9f, 0.9f, 1});

        progressBar = new Arriba::Primitives::Quad(0, 0, DIALOG_W - 80, 40, Arriba::Graphics::Pivot::topLeft);
        progressBar->transform.position = {40, 80, 0};
        progressBar->setParent(this);
        progressBar->setColour(Arriba::Colour::neutral);
        progressBar->renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "romfs:/progressBarFragment.glsl");
        progressBar->renderer->thisShader.setFloat1("aspectRatio", (float)(DIALOG_W - 80) / 40.0f);
        progressBar->renderer->thisShader.setFloat1("progress", 0.0f);

        progressText = new Arriba::Primitives::Text(U"0 / 0", 32);
        progressText->setParent(this);
        progressText->transform.position = {(float)DIALOG_W / 2, 150, 0};
        progressText->setColour({0.9f, 0.9f, 0.9f, 1});

        doneBtn = new Arriba::Elements::Button();
        doneBtn->setParent(this);
        doneBtn->setDimensions(300, 60, Arriba::Graphics::Pivot::centre);
        doneBtn->transform.position = {(float)DIALOG_W / 2, 215, 0};
        doneBtn->setText(Amiigo::Lang::get("settings_generate_all_done").c_str());
        doneBtn->enabled = false;
        doneBtn->registerCallback([this]() { closeDialog(); });
        Amiigo::UI::applySettingsQuadStyle(doneBtn);
    }

    void ProgressDialog::closeDialog() {
        Arriba::activeLayer--;
        Arriba::highlightedObject = Arriba::findObjectByName("GenerateAllAmiiboButton");
        destroy();
    }

    void ProgressDialog::onFrame() {
        if (firstFrame) {
            firstFrame = false;
            return;
        }

        int cur = progress->load(std::memory_order_relaxed);
        float fillPct = total > 0 ? (float)cur / (float)total : 0.0f;
        progressBar->renderer->thisShader.setFloat1("progress", fillPct);

        char buf[32];
        snprintf(buf, sizeof(buf), "%d / %d", cur, total);
        progressText->setText(buf);

        if (cur >= total && !doneFocusSet) {
            doneBtn->enabled = true;
            Arriba::highlightedObject = doneBtn;
            doneFocusSet = true;
        }

        if (Arriba::Input::buttonDown(Arriba::Input::backButton)) {
            if (doneBtn->enabled) closeDialog();
            return;
        }
    }
}  // namespace Amiigo::Elements
