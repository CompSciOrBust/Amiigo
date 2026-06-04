#pragma once

#include <arribaPrimitives.h>
#include <string>

namespace Amiigo::Elements {
    class AmiiboPreview : public Arriba::Primitives::Quad {
        private:
            unsigned int texID = 0;
            double spawnTime;

        public:
            AmiiboPreview(const std::string& amiiboPath);
            virtual ~AmiiboPreview();
            virtual void onFrame() override;
    };
}  // namespace Amiigo::Elements
