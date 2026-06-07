#pragma once

#include <arriba.h>

namespace Amiigo::AmiigoBehaviours {
    class StatusBarColourBehaviour : public Arriba::Behaviour {
    public:
        void update() override;
    };

    class BGBehaviour : public Arriba::Behaviour {
    public:
        void init();
        void update() override;
    };
} // namepsace Amiigo::AmiigoBehaviours
