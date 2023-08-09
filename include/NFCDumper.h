#pragma once
#include <switch.h>

namespace Amiigo::NFC::Dumper {
    // Variables
    static NfcDeviceHandle readerHandle;
    static int readerCount = 0;

    // Structs
    struct ModelFormat {
        unsigned short gameCharacterID;
        char characterVariant;
        char series;
        unsigned short modelNumber;
        char figureType;
        char unused;
    } PACKED;
    static_assert(sizeof(ModelFormat) == 8);

    // Functions
    void init();
    void exit();
    bool dumpNFC();
}