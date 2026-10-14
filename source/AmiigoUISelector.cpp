#include <AmiigoUI.h>
#include <AmiigoUISelector.h>
#include <arriba.h>
#include <utils.h>

#include <arribaElements.h>
#include <arribaText.h>
#include <emuiibo.hpp>
#include <AmiigoElements.h>
#include <AmiigoLang.h>

namespace {
    std::vector<AmiiboEntry> selectorAmiibos;
    std::string selectorPath = "sdmc:/emuiibo/amiibo";
}

namespace Amiigo::UI {
    void initSelector() {
        Arriba::Elements::InertialList* selectorList = new Arriba::Elements::InertialList(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, std::vector<std::string>{});
        selectorList->setName("SelectorList");
        selectorList->setTag("List");
        selectorList->registerCallback(selectorInput);
        selectorList->registerAltCallback(selectorContextMenuSpawner);
        updateSelectorStrings();
    }

    void handleSelectorInput() {
        if (Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && selectorPath != "sdmc:/emuiibo/amiibo") {
            selectorPath = selectorPath.substr(0, selectorPath.find_last_of("/"));
            if (selectorPath.length() < strlen("sdmc:/emuiibo/amiibo")) selectorPath = "sdmc:/emuiibo/amiibo";
            updateSelectorStrings();
        }

        if (Arriba::Input::buttonDown(Arriba::Input::XButtonSwitch)) {
            switch (emu::GetEmulationStatus()) {
                case emu::EmulationStatus::On:
                    emu::ResetActiveVirtualAmiibo();
                    emu::SetEmulationStatus(emu::EmulationStatus::Off);
                    updateStatus(Amiigo::Lang::get("status_emuiibo_disabled").c_str(), StatusLevel::Info);
                break;
                case emu::EmulationStatus::Off:
                    emu::SetEmulationStatus(emu::EmulationStatus::On);
                    updateStatus(Amiigo::Lang::get("status_emuiibo_enabled").c_str(), StatusLevel::Info);
                break;
                default:
                    updateStatus(Amiigo::Lang::get("error_unknown_emulation_status").c_str(), StatusLevel::Info);
                break;
            }
        }
    }

    void selectorInput(int index) {
        if (selectorAmiibos[index].isCategory) {
            if (checkIfFileExists(selectorAmiibos[index].path.c_str()) || selectorAmiibos[index].path == "Favorites") {
                selectorPath = selectorAmiibos[index].path;
                updateSelectorStrings();
            } else {
                updateStatus(Amiigo::Lang::get("error_folder_not_exist").c_str(), StatusLevel::Error);
            }
        } else {
            std::string path = selectorAmiibos[index].path;
            emu::SetEmulationStatus(emu::EmulationStatus::On);
            Result res = emu::SetActiveVirtualAmiibo(path.c_str(), path.size());
            if R_FAILED(res) {
                updateStatus(Amiigo::Lang::get("error_failed_set_amiibo").c_str(), StatusLevel::Error);
                return;
            }

            updateStatus(Arriba::Text::ASCIIToUnicode(path.c_str()).c_str(), StatusLevel::Info);
            Arriba::UIObject* amiiboPreview = Arriba::findObjectByName("AmiiboPreview");
            if (amiiboPreview) amiiboPreview->destroy();
            amiiboPreview = new Amiigo::Elements::AmiiboPreview(path);
            amiiboPreview->setParent(Arriba::findObjectByName("SelectorList"));
            amiiboPreview->setName("AmiiboPreview");
        }
    }

    void selectorContextMenuSpawner(int index, Arriba::Maths::vec2<float> pos) {
        if (index != -1) new Amiigo::Elements::SelectorContextMenu(static_cast<int>(pos.x), static_cast<int>(pos.y), selectorAmiibos[index]);
    }

    void updateSelectorStrings() {
        selectorAmiibos = scanForAmiibo(selectorPath.c_str());
        std::vector<std::u32string> amiiboNames;
        for (const auto& amiibo : selectorAmiibos) amiiboNames.push_back(amiibo.name);
        Arriba::findObjectByName<Arriba::Elements::InertialList>("SelectorList")->updateStrings(amiiboNames);
    }

    void resetSelectorToRoot() {
        selectorPath = "sdmc:/emuiibo/amiibo";
        updateSelectorStrings();
    }

    const std::string& getSelectorPath() {
        return selectorPath;
    }
}
