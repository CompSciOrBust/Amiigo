#include <emuiibo.hpp>

#define EMU_EMUIIBO_SERVICE_NAME "emuiibo"

namespace emu {

    namespace {

        Service g_EmuiiboService;

        inline TipcService GetSmTipcService() {
            auto sm_srv_ref = smGetServiceSession();
            return { sm_srv_ref->session };
        }

        bool smAtmosphereHasService(SmServiceName name) {
            bool has = false;
            auto sm_tipc_srv = GetSmTipcService();
            tipcDispatchInOut(&sm_tipc_srv, 65100, name, has);
            return has;
        }

    }

    bool IsAvailable() {
        return smAtmosphereHasService(smEncodeName(EMU_EMUIIBO_SERVICE_NAME));
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

    void GetVirtualAmiiboDirectory(char *out_path, size_t out_path_size) {
        serviceDispatch(&g_EmuiiboService, 1,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
            .buffers = { { out_path, out_path_size } },
        );
    }

    EmulationStatus GetEmulationStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_EmuiiboService, 2, out);
        return static_cast<EmulationStatus>(out);
    }

    void SetEmulationStatus(EmulationStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_EmuiiboService, 3, in);
    }

    Result GetActiveVirtualAmiibo(VirtualAmiiboData *out_amiibo_data, char *out_path, size_t out_path_size) {
        return serviceDispatchOut(&g_EmuiiboService, 4, *out_amiibo_data,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_Out
            },
            .buffers = {
                { out_path, out_path_size }
            },
        );
    }

    Result SetActiveVirtualAmiibo(char *path, size_t path_size) {
        return serviceDispatch(&g_EmuiiboService, 5,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
            .buffers = { { path, path_size } },
        );
    }

    void ResetActiveVirtualAmiibo() {
        serviceDispatch(&g_EmuiiboService, 6);
    }

    VirtualAmiiboStatus GetActiveVirtualAmiiboStatus() {
        u32 out = 0;
        serviceDispatchOut(&g_EmuiiboService, 7, out);
        return static_cast<VirtualAmiiboStatus>(out);
    }

    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status) {
        u32 in = static_cast<u32>(status);
        serviceDispatchIn(&g_EmuiiboService, 8, in);
    }

    void IsApplicationIdIntercepted(u64 app_id, bool *out_intercepted) {
        serviceDispatchInOut(&g_EmuiiboService, 9, app_id, *out_intercepted);
    }

    Result TryParseVirtualAmiibo(char *path, size_t path_size, VirtualAmiiboData *out_amiibo_data) {
        return serviceDispatchOut(&g_EmuiiboService, 10, *out_amiibo_data,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_In
            },
            .buffers = {
                { path, path_size }
            },
        );
    }

}