#include <emuiibo.hpp>

namespace emu {

    namespace {

        #define EMU_EMUIIBO_SERVICE_NAME "emuiibo"
        constexpr auto EmuiiboServiceName = smEncodeName(EMU_EMUIIBO_SERVICE_NAME);

        Service g_EmuiiboService;

        inline bool smAtmosphereHasService(const SmServiceName name) {
            auto has = false;
            tipcDispatchInOut(smGetServiceSessionTipc(), 65100, name, has);
            return has;
        }

    }

    bool IsAvailable() {
        return smAtmosphereHasService(EmuiiboServiceName);
    }

    Result Initialize() {
        if(serviceIsActive(&g_EmuiiboService)) {
            return 0;
        }
        return smGetService(&g_EmuiiboService, EMU_EMUIIBO_SERVICE_NAME);
    }

    void Exit() {
        serviceClose(&g_EmuiiboService);
    }

    Version GetVersion() {
        Version ver = {};
        serviceDispatchOut(&g_EmuiiboService, 0, ver);
        return ver;
    }

    void GetVirtualAmiiboDirectory(char *out_path, const size_t out_path_size) {
        serviceDispatch(&g_EmuiiboService, 1,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
            .buffers = { { out_path, out_path_size } },
        );
    }

    EmulationStatus GetEmulationStatus() {
        EmulationStatus status = EmulationStatus::Off;
        serviceDispatchOut(&g_EmuiiboService, 2, status);
        return status;
    }

    void SetEmulationStatus(const EmulationStatus status) {
        serviceDispatchIn(&g_EmuiiboService, 3, status);
    }

    Result GetActiveVirtualAmiibo(VirtualAmiiboData *out_amiibo_data, char *out_path, const size_t out_path_size) {
        return serviceDispatchOut(&g_EmuiiboService, 4, *out_amiibo_data,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_Out
            },
            .buffers = {
                { out_path, out_path_size }
            },
        );
    }

    Result SetActiveVirtualAmiibo(const char *path, const size_t path_size) {
        return serviceDispatch(&g_EmuiiboService, 5,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
            .buffers = { { path, path_size } },
        );
    }

    void ResetActiveVirtualAmiibo() {
        serviceDispatch(&g_EmuiiboService, 6);
    }

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus() {
        VirtualAmiiboStatus status = VirtualAmiiboStatus::Invalid;
        serviceDispatchOut(&g_EmuiiboService, 7, status);
        return status;
    }

    void SetActiveVirtualAmiiboStatus(const VirtualAmiiboStatus status) {
        serviceDispatchIn(&g_EmuiiboService, 8, status);
    }

    bool IsApplicationIdIntercepted(const u64 app_id) {
        bool intercepted;
        serviceDispatchInOut(&g_EmuiiboService, 9, app_id, intercepted);
        return intercepted;
    }

    Result TryParseVirtualAmiibo(const char *path, const size_t path_size, VirtualAmiiboData *out_amiibo_data) {
        return serviceDispatchOut(&g_EmuiiboService, 10, *out_amiibo_data,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_In
            },
            .buffers = {
                { path, path_size }
            },
        );
    }

    Result GetActiveVirtualAmiiboAreas(VirtualAmiiboAreaEntry *out_area_buf, const size_t out_area_size, u32 *out_area_count) {
        return serviceDispatchOut(&g_EmuiiboService, 11, *out_area_count,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_Out
            },
            .buffers = {
                { out_area_buf, out_area_size }
            },
        );
    }

    Result GetActiveVirtualAmiiboCurrentArea(u32 *out_access_id) {
        return serviceDispatchOut(&g_EmuiiboService, 12, *out_access_id);
    }

    Result SetActiveVirtualAmiiboCurrentArea(const u32 access_id) {
        return serviceDispatchIn(&g_EmuiiboService, 13, access_id);
    }

    Result SetActiveVirtualAmiiboUuidInfo(const VirtualAmiiboUuidInfo uuid_info) {
        return serviceDispatchIn(&g_EmuiiboService, 14, uuid_info);
    }

}