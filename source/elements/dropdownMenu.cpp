#include <elements/dropdownMenu.h>

namespace Amiigo::Elements {
    DropdownMenu::DropdownMenu(int x, int y, int width, const std::vector<Option>& options, int selectedIndex)
        : Arriba::Primitives::Quad(x, y, 0, 0, Arriba::Graphics::Pivot::topLeft) {
        setName("DropdownMenu");
        returnFocusTarget = Arriba::highlightedObject;
        Arriba::activeLayer++;
        setColour({0, 0, 0, 1});

        for (const auto& opt : options) {
            Arriba::Elements::Button* btn = new Arriba::Elements::Button();
            btn->setParent(this);
            btn->setText(opt.label.c_str());
            btn->setTag("DropdownMenuButton");
            btn->registerCallback([cb = opt.callback](){
                cb();
                Arriba::findObjectByName<DropdownMenu>("DropdownMenu")->closeMenu();
            });
        }

        buttonVector = Arriba::findObjectsByTag<Arriba::Elements::Button>("DropdownMenuButton");

        const int buttonHeight = 100;
        const int menuHeight = buttonHeight * (int)buttonVector.size() + 3 * ((int)buttonVector.size() + 1);

        setDimensions(width, menuHeight, Arriba::Graphics::Pivot::topLeft);
        for (unsigned int i = 0; i < buttonVector.size(); i++) {
            buttonVector[i]->setDimensions(width - 6, buttonHeight, Arriba::Graphics::Pivot::topLeft);
            buttonVector[i]->transform.position.y = static_cast<float>(i * (buttonHeight + 3) + 3);
            buttonVector[i]->transform.position.x = 3;
        }

        pendingIndex = (selectedIndex >= 0 && selectedIndex < (int)buttonVector.size()) ? selectedIndex : 0;
    }

    void DropdownMenu::onFrame() {
        if (firstFrame) {
            firstFrame = false;
            return;
        }

        if (!initialHighlightSet) {
            Arriba::highlightedObject = buttonVector[pendingIndex];
            initialHighlightSet = true;
        }

        if (Arriba::Input::buttonUp(Arriba::Input::BButtonSwitch) || Arriba::highlightedObject == nullptr) {
            closeMenu();
            return;
        }

        if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) || Arriba::Input::buttonDown(Arriba::Input::DPadUp)) {
            for (unsigned int i = 0; i < buttonVector.size(); i++) {
                if (Arriba::highlightedObject != buttonVector[i]) continue;
                if (Arriba::Input::buttonDown(Arriba::Input::DPadDown) && i != buttonVector.size() - 1) Arriba::highlightedObject = buttonVector[i + 1];
                if (Arriba::Input::buttonDown(Arriba::Input::DPadUp) && i != 0) Arriba::highlightedObject = buttonVector[i - 1];
                break;
            }
        }
    }

    void DropdownMenu::closeMenu() {
        destroy();
        Arriba::activeLayer--;
        Arriba::UIObject* parent = returnFocusTarget ? returnFocusTarget->getParent() : nullptr;
        Arriba::highlightedObject = parent ? parent : returnFocusTarget;
    }
}  // namespace Amiigo::Elements
