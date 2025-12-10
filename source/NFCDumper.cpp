#include <NFCDumper.h>
#include <utils.h>
#include <arriba.h>
#include <arribaPrimitives.h>
#include <arribaText.h>

// Goldleaf code used as implementation reference
// https://github.com/XorTroll/Goldleaf/blob/master/Goldleaf/source/nfp/nfp_Amiibo.cpp

namespace Amiigo::NFC::Dumper {
    void init() {
        nfpInitialize(NfpServiceType_Debug);
        nfpListDevices(&readerCount, &readerHandle, 1);
        nfpStartDetection(&readerHandle);
    }

    void exit() {
        nfpStopDetection(&readerHandle);
        nfpExit();
    }

    bool dumpNFC() {
        AmiiboCreatorData amiiboInfo;
        amiiboInfo.name = U"Default";
        amiiboInfo.gameName = U"Dump";
        amiiboInfo.amiiboSeries = U"Dump";

        // Mount the tag
        Result res = nfpMount(&readerHandle, NfpDeviceType_Amiibo, NfpMountTarget_All);
        // If device failed to mount
        if (!R_SUCCEEDED(res)) {
            nfpUnmount(&readerHandle);
            nfpStartDetection(&readerHandle);
            return false;
        }

        // Get the tag info
        NfpTagInfo amiiboTagInfo;
        nfpGetTagInfo(&readerHandle, &amiiboTagInfo);
        // TODO: Dump UUID

        // Get register info
        NfpRegisterInfo amiiboRegInfo;
        nfpGetRegisterInfo(&readerHandle, &amiiboRegInfo);
        // TODO: Dump mii data and get first write

        // Get common area
        NfpCommonInfo amiiboCommonArea;
        // TODO: Get last write / version

        // Get model info
        NfpModelInfo _amiiboModelInfo = {};
        nfpGetModelInfo(&readerHandle, &_amiiboModelInfo);
        // Convert from offcial NFP format to Emuiibo format
        _amiiboModelInfo.amiibo_id[5] = _amiiboModelInfo.amiibo_id[4];
        _amiiboModelInfo.amiibo_id[4] = 0;
        _amiiboModelInfo.amiibo_id[7] = 2;
        // Store model info in creator struct
        ModelFormat* amiiboModelInfo = reinterpret_cast<ModelFormat*>(_amiiboModelInfo.amiibo_id);
        amiiboInfo.game_character_id = amiiboModelInfo->gameCharacterID;
        amiiboInfo.character_variant = amiiboModelInfo->characterVariant;
        amiiboInfo.figure_type = amiiboModelInfo->figureType;
        amiiboInfo.model_number = __builtin_bswap16(amiiboModelInfo->modelNumber);
        amiiboInfo.series = amiiboModelInfo->series;

        // Get Amiibo name from user
        SwkbdConfig kbinput;
        swkbdCreate(&kbinput, 0);
        swkbdConfigMakePresetDefault(&kbinput);
        swkbdConfigSetGuideText(&kbinput, "Enter Amiibo name");
        swkbdConfigSetInitialText(&kbinput, amiiboRegInfo.amiibo_name);
        char *amiiboName = reinterpret_cast<char*>(malloc(256));
        swkbdShow(&kbinput, amiiboName, 255);
        swkbdClose(&kbinput);
        amiiboInfo.name = std::u32string(Arriba::Text::ASCIIToUnicode(amiiboName));
        free(amiiboName);

        if (amiiboInfo.name == U"") {
            nfpUnmount(&readerHandle);
            nfpStartDetection(&readerHandle);
            static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Dump failed (No name provided)");
            return false;
        }

        // Create the virtual amiibo
        createVirtualAmiibo(amiiboInfo);
        static_cast<Arriba::Primitives::Text*>(Arriba::findObjectByName("StatusBarText"))->setText("Amiibo dumped to SD");
        nfpUnmount(&readerHandle);
        nfpStartDetection(&readerHandle);
        return true;
    }
}  // namespace Amiigo::NFC::Dumper
