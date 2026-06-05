#include <elements/amiiboPreview.h>
#include <AmiigoUI.h>
#include <utils.h>

#include <glad/glad.h>
#include <stb_image.h>

namespace {
    constexpr double displayDuration = 5.0;
    constexpr double fadeDuration    = 0.5;
}

namespace Amiigo::Elements {
    AmiiboPreview::AmiiboPreview(const std::string& amiiboPath)
        : Arriba::Primitives::Quad(
            Arriba::Graphics::windowWidth - Amiigo::UI::switcherWidth,
            (Arriba::Graphics::windowHeight - Amiigo::UI::statusHeight)/2,
            200, 200,
            Arriba::Graphics::Pivot::centre
        )
    {
        spawnTime = Arriba::time;
        std::string imagePath = amiiboPath + "/amiibo.png";
        if (checkIfFileExists(imagePath.c_str())) {
            int w, h, channels;
            unsigned char* data = stbi_load(imagePath.c_str(), &w, &h, &channels, 4);
            if (data) {
                int outW = w, outH = h;
                auto outData = scaleImageToFit(data, w, h, 4, 200, outW, outH);

                this->transform.position.x -= (outW/2 + 10);
                setDimensions(outW, outH, Arriba::Graphics::Pivot::centre);
                texID = Arriba::Graphics::bufferTexture_RGBA(outW, outH, outData.data());
                renderer->setTexture(texID);

                stbi_image_free(data);
            }
        } else {
            spawnTime = Arriba::time - (displayDuration + fadeDuration);
        }
    }

    AmiiboPreview::~AmiiboPreview() {
        if (texID != 0) glDeleteTextures(1, &texID);
    }

    void AmiiboPreview::onFrame() {
        double elapsed = Arriba::time - spawnTime;
        if (elapsed >= displayDuration + fadeDuration) {
            Amiigo::UI::amiiboPreview = nullptr;
            destroy();
            return;
        }
        float alpha = (elapsed >= displayDuration) ? 1.0f - (float)((elapsed - displayDuration) / fadeDuration) : 1.0f;
        setColour({1.0f, 1.0f, 1.0f, alpha});
    }
}  // namespace Amiigo::Elements
