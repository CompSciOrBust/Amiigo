#include <AmiigoBehaviours.h>
#include <AmiigoSettings.h>
#include <utils.h>

namespace Amiigo::AmiigoBehaviours {
    void StatusBarColourBehaviour::update() {
        object->setColour(Arriba::Maths::lerp(object->getColour(), Amiigo::Settings::Colour::statusBar, Arriba::deltaTime * 2));
    }

    void BGBehaviour::init() {
        if(checkIfFileExists("sdmc:/config/amiigo/bgFragment.glsl")) object->renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "sdmc:/config/amiigo/bgFragment.glsl");
        else object->renderer->thisShader.updateFragments("romfs:/VertexDefault.glsl", "romfs:/bgFragment.glsl");
    }

    void BGBehaviour::update() {
        object->renderer->thisShader.setFloat1("iTime", Arriba::time);
    }
} // namespace Amiigo::AmiigoBehaviours
