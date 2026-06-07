#pragma once

#include <switch.h>

namespace Amiigo::NFC::Dumper {
    struct ModelFormat {
        unsigned short gameCharacterID;
        char characterVariant;
        char series;
        unsigned short modelNumber;
        char figureType;
        char unused;
    } NX_PACKED;
    static_assert(sizeof(ModelFormat) == 8);

    void init();
    void exit();
    bool poll();
    bool dumpNFC();
}  // namespace Amiigo::NFC::Dumper
