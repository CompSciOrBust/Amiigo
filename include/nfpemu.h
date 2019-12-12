
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

bool nfpemuIsAccessible();

Result nfpemuInitialize();
void nfpemuExit();

typedef enum {
    EmuEmulationStatus_OnForever = 0,
    EmuEmulationStatus_OnOnce = 1,
    EmuEmulationStatus_Off = 2,
} NfpEmuEmulationStatus;

typedef struct {
    u8 major;
    u8 minor;
    u8 micro;
    u8 dev_build;
} NfpEmuVersion;

enum {
    Module_NfpEmu = 352
};

enum {
    NfpEmuError_NoAmiiboLoaded = 1,
    NfpEmuError_UnableToMove = 2,
    NfpEmuError_StatusOff = 3
};

Result nfpemuGetCurrentAmiibo(char *out);
Result nfpemuSetCustomAmiibo(const char *path);
Result nfpemuHasCustomAmiibo(bool *out_has);
Result nfpemuResetCustomAmiibo();
Result nfpemuSetEmulationOnForever();
Result nfpemuSetEmulationOnOnce();
Result nfpemuSetEmulationOff();
Result nfpemuMoveToNextAmiibo();
Result nfpemuGetStatus(NfpEmuEmulationStatus *out);
Result nfpemuRefresh();
Result nfpemuGetVersion(NfpEmuVersion *out_ver);

NX_CONSTEXPR bool nfpemuIsDevBuild(const NfpEmuVersion *ver) {
    return !!ver->dev_build;
}

NX_CONSTEXPR bool nfpemuMatches(const NfpEmuVersion *ver, u8 major, u8 minor, u8 micro) {
    return (ver->major == major) && (ver->minor == minor) && (ver->micro == micro);
}

#ifdef __cplusplus
}
#endif