
#pragma once
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

        inline bool IsValid() {
            return strlen(this->name) > 0;
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

        inline constexpr bool EqualsExceptBuild(const Version &other) {
            return (other.major == this->major) && (other.minor == this->minor) && (other.micro == this->micro);
        }
    };

    bool IsAvailable();

    Result Initialize();
    void Exit();

    Version GetVersion();

    void GetVirtualAmiiboDirectory(char *out_path, const size_t out_path_size);

    EmulationStatus GetEmulationStatus();
    void SetEmulationStatus(const EmulationStatus status);

    Result GetActiveVirtualAmiibo(VirtualAmiiboData *out_amiibo_data, char *out_path, size_t out_path_size);
    Result SetActiveVirtualAmiibo(const char *path, const size_t path_size);
    void ResetActiveVirtualAmiibo();

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus();
    void SetActiveVirtualAmiiboStatus(const VirtualAmiiboStatus status);

    bool IsApplicationIdIntercepted(const u64 app_id);

    inline bool IsCurrentApplicationIdIntercepted() {
        bool intercepted = false;
        u64 process_id = 0;
        if(R_SUCCEEDED(pmdmntGetApplicationProcessId(&process_id))) {
            u64 program_id = 0;
            if(R_SUCCEEDED(pmdmntGetProgramId(&program_id, process_id))) {
                intercepted = IsApplicationIdIntercepted(program_id);
            }
        }
        return intercepted;
    }

    Result TryParseVirtualAmiibo(const char *path, const size_t path_size, VirtualAmiiboData *out_amiibo_data);
    Result GetActiveVirtualAmiiboAreas(VirtualAmiiboAreaEntry *out_area_buf, const size_t out_area_size, u32 *out_area_count);
    Result GetActiveVirtualAmiiboCurrentArea(u32 *out_access_id);
    Result SetActiveVirtualAmiiboCurrentArea(const u32 access_id);
    Result SetActiveVirtualAmiiboUuidInfo(const VirtualAmiiboUuidInfo uuid_info);

}