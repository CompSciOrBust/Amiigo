#include <AmiigoElements.h>
#include <AmiigoUI.h>
#include <sys/stat.h>
#include <fstream>
#include <vector>

namespace Amiigo::Elements {
    selectorContextMenu::selectorContextMenu(int x, int y, AmiiboEntry entry) : Arriba::Primitives::Quad(x, y, 0, 0, Arriba::Graphics::Pivot::centre) {
        name = "ContextMenu";
        Arriba::activeLayer++;
        setColour({0, 0, 0, 1});
        amiiboEntryGlobal = entry;
        // Spawn favorite button
        if (entry.name != "¤ Favorites" && entry.name != "¤ Back") {
            Arriba::Elements::Button* favoriteButton = new Arriba::Elements::Button();
            favoriteButton->setParent(this);
            // Check if the amiibo is currently favorited
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
            favoriteButton->tag = "ContextMenuButton";
            // Handle adding amiibo to favorites
            if (!isFavorited) {
                favoriteButton->registerCallback([](){
                    mkdir("sdmc:/emuiibo/overlay/", 0);
                    std::ofstream favFile("sdmc:/emuiibo/overlay/favorites.txt", std::ofstream::app);
                    favFile << Amiigo::Elements::amiiboEntryGlobal.path << "\n";
                    favFile.close();
                    // Close the menu
                    Arriba::highlightedObject = nullptr;
                });
            } else {
                // Handle removing amiibo from favorites
                favoriteButton->registerCallback([](){
                    // Backup all paths to a vector, except for the one we're unfavoriting
                    std::string tempLine;
                    std::ifstream favFile("sdmc:/emuiibo/overlay/favorites.txt");
                    std::vector<std::string> amiiboPaths;
                    while (getline(favFile, tempLine)) if (Amiigo::Elements::amiiboEntryGlobal.path != tempLine) amiiboPaths.push_back(tempLine);
                    favFile.close();
                    // Write the vector back
                    std::ofstream reFavFile("sdmc:/emuiibo/overlay/favorites.txt", std::ofstream::trunc);
                    for (unsigned int i = 0; i < amiiboPaths.size(); i++) {
                        reFavFile << amiiboPaths[i] << "\n";
                    }
                    reFavFile.close();
                    // Close menu
                    Amiigo::UI::updateSelectorStrings();
                    Arriba::highlightedObject = nullptr;
                });
            }
        }
        // Spawn new folder button
        if (Amiigo::UI::selectorPath != "Favorites") {
            Arriba::Elements::Button* newFolderButton = new Arriba::Elements::Button();
            newFolderButton->setParent(this);
            newFolderButton->setText("New folder");
            newFolderButton->tag = "ContextMenuButton";
            // If pressed create a new folder
            newFolderButton->registerCallback([](){
                // Get folder name from on screen keyboard
                SwkbdConfig kbinput;
                swkbdCreate(&kbinput, 0);
                swkbdConfigMakePresetDefault(&kbinput);
                swkbdConfigSetGuideText(&kbinput, "Enter folder name");
                swkbdConfigSetInitialText(&kbinput, "New folder");
                char *kbout = reinterpret_cast<char*>(malloc(256));
                swkbdShow(&kbinput, kbout, 255);
                swkbdClose(&kbinput);
                std::string newFolderPath = Amiigo::UI::selectorPath + "/" + kbout;
                printf("%s\n", newFolderPath.c_str());
                mkdir(newFolderPath.c_str(), 0);
                free(kbout);
                // Refresh list and close menu
                Amiigo::UI::updateSelectorStrings();
                Arriba::highlightedObject = nullptr;
            });
        }
        // Spawn delete button
        if (entry.name != "¤ Favorites" && Amiigo::UI::selectorPath != "Favorites" && entry.name != "¤ Back") {
            Arriba::Elements::Button* deleteButton = new Arriba::Elements::Button();
            deleteButton->setParent(this);
            deleteButton->setText("Delete");
            deleteButton->tag = "ContextMenuButton";
            // If pressed delete the current entry and reload the list
            deleteButton->registerCallback([](){
                fsdevDeleteDirectoryRecursively(amiiboEntryGlobal.path.c_str());
                Amiigo::UI::updateSelectorStrings();
                // Results in program abort. Idk why. Set highlighted to nullptr as workaround to close menu.
                // static_cast<Amiigo::Elements::selectorContextMenu*>(Arriba::findObjectByName("ContextMenu"))->destroy();
                Arriba::highlightedObject = nullptr;
            });
        }
        // If no buttons were spawned then destory self
        buttonVector = Arriba::findObjectsByTag("ContextMenuButton");
        if (buttonVector.size() == 0) closeMenu();
        // Resize menu
		int menuHeight = 103 * buttonVector.size() + 3;
        int menuWidth = 306;
		bool tooFarX = false;
		bool tooFarY = false;
		tooFarX = transform.position.x + menuWidth > Arriba::Graphics::windowWidth;
		tooFarY = transform.position.y + menuHeight > Arriba::Graphics::windowHeight;
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
        // Position buttons
        for (unsigned int i = 0; i < buttonVector.size(); i++) {
            static_cast<Arriba::Primitives::Quad*>(buttonVector[i])->setDimensions(menuWidth-6, 100, Arriba::Graphics::Pivot::topLeft);
            buttonVector[i]->transform.position.y = static_cast<float>(i*100 + (i+1)*3) - topYDelta;
            buttonVector[i]->transform.position.x = 3;
            Arriba::highlightedObject = buttonVector[0];
        }
    }

    void selectorContextMenu::onFrame() {
        // We have to use buttonUp to detect B being pressed otherwise handleInput from AmiigoUI.h wil force the selector to go up a directory
        if ((Arriba::Input::touch.end && Arriba::highlightedObject == nullptr) || Arriba::Input::buttonUp(Arriba::Input::BButtonSwitch) || Arriba::highlightedObject == nullptr) {
            closeMenu();
            return;
        }
        // Check if up / down is pressed
        if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) || Arriba::Input::buttonDown(Arriba::Input::DPadUp)) {
            for (unsigned int i = 0; i < buttonVector.size(); i++) {
                if (Arriba::highlightedObject == buttonVector[i]) {
                    if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) && i != buttonVector.size()-1) {
                        Arriba::highlightedObject = buttonVector[i+1];
                        // We don't want to repeatedly do this for each item in a single frame
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
