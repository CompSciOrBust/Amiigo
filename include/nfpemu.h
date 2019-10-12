
#include <switch.h>

#ifdef __cplusplus
extern "C" {
#endif

bool emuiiboIsPresent();

Result nfpemuInitialize();
void nfpemuExit();

typedef enum {
    EmuEmulationStatus_OnForever = 0,
    EmuEmulationStatus_OnOnce = 1,
    EmuEmulationStatus_Off = 2,
} EmuEmulationStatus;

typedef struct {
    u8 Major;
    u8 Minor;
    u8 Micro;
} EmuVersion;

Result nfpemuGetCurrentAmiibo(char *out, bool *out_ok);
Result nfpemuSetCustomAmiibo(const char *path);
Result nfpemuHasCustomAmiibo(bool *out_has);
Result nfpemuResetCustomAmiibo();
Result nfpemuSetEmulationOnForever();
Result nfpemuSetEmulationOnOnce();
Result nfpemuSetEmulationOff();
Result nfpemuMoveToNextAmiibo(bool *out_ok);
Result nfpemuGetStatus(EmuEmulationStatus *out);
Result nfpemuRefresh();
Result nfpemuGetVersion(EmuVersion *out_ver);

#ifdef __cplusplus
}
#endif