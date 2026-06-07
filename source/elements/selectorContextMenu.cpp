#include <elements/selectorContextMenu.h>
#include <AmiigoUI.h>
#include <sys/stat.h>
#include <fstream>
#include <vector>

namespace Amiigo::Elements {
    selectorContextMenu::selectorContextMenu(int x, int y, AmiiboEntry entry) : Arriba::Primitives::Quad(x, y, 0, 0, Arriba::Graphics::Pivot::centre) {
        setName("ContextMenu");
        Arriba::activeLayer++;
        setColour({0, 0, 0, 1});
        amiiboEntryGlobal = entry;

        if (entry.name != U"★Favorites" && entry.name != U"← Back") {
            Arriba::Elements::Button* favoriteButton = new Arriba::Elements::Button();
            favoriteButton->setParent(this);
            std::string tempLine;
            std::ifstream fileStream("sdmc:/emuiibo/overlay/favorites.txt");
            bool isFavorited = false;
            while (getline(fileStream, tempLine)) {
                if (tempLine == entry.path) {
                    isFavorited = true;
                    break;
                }
            }
            if (!isFavorited)
                favoriteButton->setText("Favorite");
            else
                favoriteButton->setText("Unfavorite");
            favoriteButton->setTag("ContextMenuButton");
            if (!isFavorited) {
                favoriteButton->registerCallback([](){
                    mkdir("sdmc:/emuiibo/overlay/", 0);
                    std::ofstream favFile("sdmc:/emuiibo/overlay/favorites.txt", std::ofstream::app);
                    favFile << Amiigo::Elements::amiiboEntryGlobal.path << "\n";
                    favFile.close();
                    Arriba::findObjectByName<selectorContextMenu>("ContextMenu")->closeMenu();
                });
            } else {
                favoriteButton->registerCallback([](){
                    std::string tempLine;
                    std::ifstream favFile("sdmc:/emuiibo/overlay/favorites.txt");
                    std::vector<std::string> amiiboPaths;
                    while (getline(favFile, tempLine)) if (Amiigo::Elements::amiiboEntryGlobal.path != tempLine) amiiboPaths.push_back(tempLine);
                    favFile.close();
                    std::ofstream reFavFile("sdmc:/emuiibo/overlay/favorites.txt", std::ofstream::trunc);
                    for (unsigned int i = 0; i < amiiboPaths.size(); i++) {
                        reFavFile << amiiboPaths[i] << "\n";
                    }
                    reFavFile.close();
                    Amiigo::UI::updateSelectorStrings();
                    Arriba::findObjectByName<selectorContextMenu>("ContextMenu")->closeMenu();
                });
            }
        }

        if (Amiigo::UI::getSelectorPath() != "Favorites") {
            Arriba::Elements::Button* newFolderButton = new Arriba::Elements::Button();
            newFolderButton->setParent(this);
            newFolderButton->setText("New folder");
            newFolderButton->setTag("ContextMenuButton");
            newFolderButton->registerCallback([](){
                SwkbdConfig kbinput;
                swkbdCreate(&kbinput, 0);
                swkbdConfigMakePresetDefault(&kbinput);
                swkbdConfigSetGuideText(&kbinput, "Enter folder name");
                swkbdConfigSetInitialText(&kbinput, "New folder");
                char kbout[256];
                swkbdShow(&kbinput, kbout, 255);
                swkbdClose(&kbinput);
                std::string newFolderPath = Amiigo::UI::getSelectorPath() + "/" + kbout;
                mkdir(newFolderPath.c_str(), 0);
                Amiigo::UI::updateSelectorStrings();
                Arriba::findObjectByName<selectorContextMenu>("ContextMenu")->closeMenu();
            });
        }

        if (entry.name != U"★Favorites" && Amiigo::UI::getSelectorPath() != "Favorites" && entry.name != U"← Back") {
            Arriba::Elements::Button* deleteButton = new Arriba::Elements::Button();
            deleteButton->setParent(this);
            deleteButton->setText("Delete");
            deleteButton->setTag("ContextMenuButton");
            deleteButton->registerCallback([](){
                fsdevDeleteDirectoryRecursively(amiiboEntryGlobal.path.c_str());
                Amiigo::UI::updateSelectorStrings();
                Arriba::findObjectByName<selectorContextMenu>("ContextMenu")->closeMenu();
            });
        }

        buttonVector = Arriba::findObjectsByTag<Arriba::Elements::Button>("ContextMenuButton");
        if (buttonVector.size() == 0) closeMenu();
		int menuHeight = 103 * buttonVector.size() + 3;
        int menuWidth = 306;
		bool tooFarX = transform.position.x + menuWidth > Arriba::Graphics::windowWidth;
		bool tooFarY = transform.position.y + menuHeight > Arriba::Graphics::windowHeight;
        Arriba::Graphics::Pivot pivotMode = Arriba::Graphics::Pivot::centre;

		if (tooFarX && tooFarY) {
            pivotMode = Arriba::Graphics::Pivot::bottomRight;
        } else if (tooFarX && !tooFarY) {
            pivotMode = Arriba::Graphics::Pivot::topRight;
        } else if (!tooFarX && tooFarY) {
            pivotMode = Arriba::Graphics::Pivot::bottomLeft;
        } else {
            pivotMode = Arriba::Graphics::Pivot::topLeft;
        }

        setDimensions(menuWidth, menuHeight, pivotMode);
        int topYDelta = menuHeight - top;
        for (unsigned int i = 0; i < buttonVector.size(); i++) {
            buttonVector[i]->setDimensions(menuWidth-6, 100, Arriba::Graphics::Pivot::topLeft);
            buttonVector[i]->transform.position.y = static_cast<float>(i*100 + (i+1)*3) - topYDelta;
            buttonVector[i]->transform.position.x = 3;
        }
        Arriba::highlightedObject = buttonVector[0];
    }

    void selectorContextMenu::onFrame() {
        // We have to use buttonUp to detect B being pressed otherwise handleInput from AmiigoUI.h will force the selector to go up a directory
        if ((Arriba::Input::touch.end && Arriba::highlightedObject == nullptr) || Arriba::Input::buttonUp(Arriba::Input::BButtonSwitch) || Arriba::highlightedObject == nullptr) {
            closeMenu();
            return;
        }
        
        if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) || Arriba::Input::buttonDown(Arriba::Input::DPadUp)) {
            for (unsigned int i = 0; i < buttonVector.size(); i++) {
                if (Arriba::highlightedObject == buttonVector[i]) {
                    if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) && i != buttonVector.size()-1) {
                        Arriba::highlightedObject = buttonVector[i+1];
                        break;
                    }
                    if (Arriba::Input::buttonDown(Arriba::Input::DPadUp) && i != 0) Arriba::highlightedObject = buttonVector[i-1];
                }
            }
        }
    }

    void selectorContextMenu::closeMenu() {
        destroy();
        Arriba::activeLayer--;
        Arriba::highlightedObject = Arriba::findObjectByName("SelectorList");
    }
}  // namespace Amiigo::Elements
