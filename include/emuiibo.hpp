
#pragma once
#include <vector>
#include <string>
#include <switch.h>
#include <cstring>

namespace emu {

    struct VirtualAmiiboUuidInfo {
        bool random_uuid;
        u8 uuid[10];
    };

    struct VirtualAmiiboDate {
        u16 year;
        u8 month;
        u8 day;
    };

    struct VirtualAmiiboData {
        VirtualAmiiboUuidInfo uuid;
        char name[40 + 1];
        VirtualAmiiboDate first_write_date;
        VirtualAmiiboDate last_write_date;
        MiiCharInfo mii_charinfo;

        inline bool IsValid() {
            return strlen(this->name);
        }

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
    };

    bool IsAvailable();

    Result Initialize();
    void Exit();

    Version GetVersion();

    void GetVirtualAmiiboDirectory(char *out_path, size_t out_path_size);

    EmulationStatus GetEmulationStatus();
    void SetEmulationStatus(EmulationStatus status);

    Result GetActiveVirtualAmiibo(VirtualAmiiboData *out_amiibo_data, char *out_path, size_t out_path_size);
    Result SetActiveVirtualAmiibo(char *path, size_t path_size);
    void ResetActiveVirtualAmiibo();

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus();
    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status);

    void IsApplicationIdIntercepted(u64 app_id, bool *out_intercepted);

    inline bool IsCurrentApplicationIdIntercepted() {
        bool intercepted = false;
        u64 process_id = 0;
        if(R_SUCCEEDED(pmdmntGetApplicationProcessId(&process_id))) {
            u64 program_id = 0;
            if(R_SUCCEEDED(pminfoGetProgramId(&program_id, process_id))) {
                IsApplicationIdIntercepted(program_id, &intercepted);
            }
        }
        return intercepted;
    }

    Result TryParseVirtualAmiibo(char *path, size_t path_size, VirtualAmiiboData *out_amiibo_data);

}