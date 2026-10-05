#include <elements/makerContextMenu.h>
#include <AmiigoUI.h>
#include <AmiigoLang.h>
#include <AmiigoSettings.h>
#include <WorkerQueue.h>
#include <elements/progressDialog.h>
#include <atomic>
#include <memory>
#include <vector>

namespace Amiigo::Elements {
    MakerContextMenu::MakerContextMenu(int x, int y, const std::string& series) : Arriba::Primitives::Quad(x, y, 0, 0, Arriba::Graphics::Pivot::centre) {
        setName("MakerContextMenu");
        Arriba::activeLayer++;
        setColour({0, 0, 0, 1});

        Arriba::Elements::Button* createAllButton = new Arriba::Elements::Button();
        createAllButton->setParent(this);
        createAllButton->setText(Amiigo::Lang::get("context_create_all").c_str());
        createAllButton->setTag("MakerContextMenuButton");
        createAllButton->registerCallback([series]() {
            auto amiibos = getAmiibosFromSeries(series);
            int total = (int)amiibos.size();
            if (total == 0) {
                Amiigo::UI::updateStatus(Amiigo::Lang::get("error_no_api_cache").c_str(), Amiigo::UI::StatusLevel::Error);
                Arriba::findObjectByName<MakerContextMenu>("MakerContextMenu")->closeMenu();
                return;
            }
            auto progress = std::make_shared<std::atomic<int>>(0);
            const int makerW = Arriba::Graphics::windowWidth - Amiigo::UI::switcherWidth - 1;
            Arriba::findObjectByName<MakerContextMenu>("MakerContextMenu")->closeMenu();
            new Amiigo::Elements::ProgressDialog(
                (makerW - Amiigo::Elements::ProgressDialog::DIALOG_W) / 2,
                Amiigo::UI::statusHeight + (Arriba::Graphics::windowHeight - Amiigo::UI::statusHeight - Amiigo::Elements::ProgressDialog::DIALOG_H) / 2,
                Amiigo::Lang::get("settings_generate_all_progress").c_str(),
                progress, total
            );
            bool downloadImages = Amiigo::Settings::saveAmiiboImages;
            workerQueue.enqueue([amiibos = std::move(amiibos), progress, downloadImages]() {
                for (const auto& amiibo : amiibos) {
                    std::string pathBase = createVirtualAmiibo(amiibo, false);
                    if (downloadImages) saveAmiiboImage(pathBase, amiibo);
                    progress->fetch_add(1, std::memory_order_relaxed);
                }
                MainThread::dispatch([]() {
                    Amiigo::UI::updateSelectorStrings();
                    Amiigo::UI::updateStatus(Amiigo::Lang::get("status_generate_all_complete").c_str(), Amiigo::UI::StatusLevel::Info);
                });
            });
        });

        buttonVector = Arriba::findObjectsByTag<Arriba::Elements::Button>("MakerContextMenuButton");
        if (buttonVector.size() == 0) { closeMenu(); return; }
        int menuHeight = 103 * buttonVector.size() + 3;
        int menuWidth = 306;
        bool tooFarX = transform.position.x + menuWidth > Arriba::Graphics::windowWidth;
        bool tooFarY = transform.position.y + menuHeight > Arriba::Graphics::windowHeight;
        Arriba::Graphics::Pivot pivotMode = Arriba::Graphics::Pivot::topLeft;
        if (tooFarX && tooFarY) pivotMode = Arriba::Graphics::Pivot::bottomRight;
        else if (tooFarX) pivotMode = Arriba::Graphics::Pivot::topRight;
        else if (tooFarY) pivotMode = Arriba::Graphics::Pivot::bottomLeft;

        setDimensions(menuWidth, menuHeight, pivotMode);
        int topYDelta = menuHeight - top;
        for (unsigned int i = 0; i < buttonVector.size(); i++) {
            buttonVector[i]->setDimensions(menuWidth - 6, 100, Arriba::Graphics::Pivot::topLeft);
            buttonVector[i]->transform.position.y = static_cast<float>(i * 100 + (i + 1) * 3) - topYDelta;
            buttonVector[i]->transform.position.x = 3;
        }
        Arriba::highlightedObject = buttonVector[0];
    }

    void MakerContextMenu::onFrame() {
        if ((Arriba::Input::touch.end && Arriba::highlightedObject == nullptr) || Arriba::Input::buttonUp(Arriba::Input::BButtonSwitch) || Arriba::highlightedObject == nullptr) {
            closeMenu();
            return;
        }
        if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) || Arriba::Input::buttonDown(Arriba::Input::DPadUp)) {
            for (unsigned int i = 0; i < buttonVector.size(); i++) {
                if (Arriba::highlightedObject == buttonVector[i]) {
                    if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) && i != buttonVector.size() - 1) Arriba::highlightedObject = buttonVector[i + 1];
                    if (Arriba::Input::buttonDown(Arriba::Input::DPadUp) && i != 0) Arriba::highlightedObject = buttonVector[i - 1];
                    break;
                }
            }
        }
    }

    void MakerContextMenu::closeMenu() {
        destroy();
        Arriba::activeLayer--;
        Arriba::highlightedObject = Arriba::findObjectByName("MakerList");
    }
}  // namespace Amiigo::Elements
