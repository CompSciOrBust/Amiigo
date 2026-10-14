#include <AmiigoUI.h>
#include <AmiigoUIMaker.h>
#include <arriba.h>
#include <utils.h>

#include <arribaElements.h>
#include <AmiigoElements.h>

namespace {
    std::vector<std::string> seriesList;
    std::vector<AmiiboCreatorData> creatorData;
    bool makerIsInCategory = false;
}

namespace Amiigo::UI {
    void initMaker() {
        seriesList = getListOfSeries();
        Arriba::Elements::InertialList* makerList = new Arriba::Elements::InertialList(0, statusHeight, Arriba::Graphics::windowWidth - switcherWidth - 1, Arriba::Graphics::windowHeight - statusHeight, seriesList);
        makerList->registerCallback(makerInput);
        makerList->registerAltCallback(makerContextMenuSpawner);
        makerList->setName("MakerList");
        makerList->setTag("List");
        makerList->enabled = false;
    }

    void handleMakerInput() {
        if (Arriba::Input::buttonDown(Arriba::Input::BButtonSwitch) && makerIsInCategory) {
            Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList")->updateStrings(seriesList);
            makerIsInCategory = false;
        }
    }

    void makerInput(int index) {
        if (makerIsInCategory) {
            if (index == 0) {
                Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList")->updateStrings(seriesList);
                makerIsInCategory = false;
            } else {
                createVirtualAmiibo(creatorData[index-1]);
                updateStatus((U"Created " + creatorData[index-1].name).c_str(), StatusLevel::Info);
            }
        } else {
            creatorData = getAmiibosFromSeries(seriesList[index]);
            std::vector<std::u32string> amiiboNames = {U"← Back"};
            for (const auto& data : creatorData) amiiboNames.push_back(data.name);
            Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList")->updateStrings(amiiboNames);
            makerIsInCategory = true;
        }
    }

    void resetMakerToRoot() {
        makerIsInCategory = false;
        Arriba::findObjectByName<Arriba::Elements::InertialList>("MakerList")->updateStrings(seriesList);
    }

    void makerContextMenuSpawner(int index, Arriba::Maths::vec2<float> pos) {
        if (index != -1 && !makerIsInCategory) new Amiigo::Elements::MakerContextMenu(static_cast<int>(pos.x), static_cast<int>(pos.y), seriesList[index]);
    }
}
