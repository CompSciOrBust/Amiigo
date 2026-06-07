#pragma once
// https://github.com/XorTroll/emuiibo/blob/master/overlay/include/emu/emu_Service.hpp
// Style modifications applied.
#include <switch.h>
#include <cstring>

namespace emu {

    struct VirtualAmiiboUuidInfo {
        bool use_random_uuid;
        u8 uuid[10];
    };

    struct VirtualAmiiboDate {
        u16 year;
        u8 month;
        u8 day;
    };

    struct VirtualAmiiboData {
        VirtualAmiiboUuidInfo uuid_info;
        char name[40 + 1];
        VirtualAmiiboDate first_write_date;
        VirtualAmiiboDate last_write_date;
        MiiCharInfo mii_charinfo;

        bool IsValid() {
            return strlen(name) > 0;
        }
    };

    struct VirtualAmiiboAreaEntry {
        u64 program_id;
        u32 access_id;
    };

    enum class EmulationStatus : u32 {
        On,
        Off,
    };

    enum class VirtualAmiiboStatus : u32 {
        Invalid,
        Connected,
        Disconnected
    };

    struct Version {
        u8 major;
        u8 minor;
        u8 micro;
        bool dev_build;

        constexpr bool EqualsExceptBuild(const Version& other) {
            return (other.major == major) && (other.minor == minor) && (other.micro == micro);
        }
    };

    bool IsAvailable();

    Result Initialize();
    void Exit();

    Version GetVersion();

    void GetVirtualAmiiboDirectory(char* outPath, size_t outPathSize);

    EmulationStatus GetEmulationStatus();
    void SetEmulationStatus(EmulationStatus status);

    Result GetActiveVirtualAmiibo(VirtualAmiiboData* outAmiiboData, char* outPath, size_t outPathSize);
    Result SetActiveVirtualAmiibo(const char* path, size_t pathSize);
    void ResetActiveVirtualAmiibo();

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus();
    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status);

    bool IsApplicationIdIntercepted(u64 appId);

    inline bool IsCurrentApplicationIdIntercepted() {
        bool intercepted = false;
        u64 processId = 0;
        if (R_SUCCEEDED(pmdmntGetApplicationProcessId(&processId))) {
            u64 programId = 0;
            if (R_SUCCEEDED(pmdmntGetProgramId(&programId, processId))) {
                intercepted = IsApplicationIdIntercepted(programId);
            }
        }
        return intercepted;
    }

    Result TryParseVirtualAmiibo(const char* path, size_t pathSize, VirtualAmiiboData* outAmiiboData);
    Result GetActiveVirtualAmiiboAreas(VirtualAmiiboAreaEntry* outAreaBuf, size_t outAreaSize, u32* outAreaCount);
    Result GetActiveVirtualAmiiboCurrentArea(u32* outAccessId);
    Result SetActiveVirtualAmiiboCurrentArea(u32 accessId);
    Result SetActiveVirtualAmiiboUuidInfo(VirtualAmiiboUuidInfo uuidInfo);

}  // namespace emu
