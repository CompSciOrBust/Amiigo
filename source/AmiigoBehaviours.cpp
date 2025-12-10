#include <AmiigoBehaviours.h>
#include <AmiigoSettings.h>

namespace Amiigo::AmiigoBehaviours {
    void StatusBarColourBehaviour::update() {
        object->setColour(Arriba::Maths::lerp(object->getColour(), Amiigo::Settings::Colour::statusBar, Arriba::deltaTime * 2));
    }
} // namepsace Amiigo::AmiigoBehaviours
