#include <emuiibo.hpp>
// https://github.com/XorTroll/emuiibo/blob/master/overlay/source/emu/emu_Service.cpp
// Style modifications applied.

namespace emu {

    namespace {
        constexpr auto EmuiiboServiceName = smEncodeName("emuiibo");

        Service g_EmuiiboService;

        bool smAtmosphereHasService(const SmServiceName name) {
            bool has = false;
            tipcDispatchInOut(smGetServiceSessionTipc(), 65100, name, has);
            return has;
        }

    }

    bool IsAvailable() {
        return smAtmosphereHasService(EmuiiboServiceName);
    }

    Result Initialize() {
        if (serviceIsActive(&g_EmuiiboService)) {
            return 0;
        }
        return smGetService(&g_EmuiiboService, "emuiibo");
    }

    void Exit() {
        serviceClose(&g_EmuiiboService);
    }

    Version GetVersion() {
        Version ver = {};
        serviceDispatchOut(&g_EmuiiboService, 0, ver);
        return ver;
    }

    void GetVirtualAmiiboDirectory(char* outPath, size_t outPathSize) {
        serviceDispatch(&g_EmuiiboService, 1,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
            .buffers = { { outPath, outPathSize } },
        );
    }

    EmulationStatus GetEmulationStatus() {
        EmulationStatus status = EmulationStatus::Off;
        serviceDispatchOut(&g_EmuiiboService, 2, status);
        return status;
    }

    void SetEmulationStatus(EmulationStatus status) {
        serviceDispatchIn(&g_EmuiiboService, 3, status);
    }

    Result GetActiveVirtualAmiibo(VirtualAmiiboData* outAmiiboData, char* outPath, size_t outPathSize) {
        return serviceDispatchOut(&g_EmuiiboService, 4, *outAmiiboData,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_Out
            },
            .buffers = {
                { outPath, outPathSize }
            },
        );
    }

    Result SetActiveVirtualAmiibo(const char* path, size_t pathSize) {
        return serviceDispatch(&g_EmuiiboService, 5,
            .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
            .buffers = { { path, pathSize } },
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

    void SetActiveVirtualAmiiboStatus(VirtualAmiiboStatus status) {
        serviceDispatchIn(&g_EmuiiboService, 8, status);
    }

    bool IsApplicationIdIntercepted(u64 appId) {
        bool intercepted;
        serviceDispatchInOut(&g_EmuiiboService, 9, appId, intercepted);
        return intercepted;
    }

    Result TryParseVirtualAmiibo(const char* path, size_t pathSize, VirtualAmiiboData* outAmiiboData) {
        return serviceDispatchOut(&g_EmuiiboService, 10, *outAmiiboData,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_In
            },
            .buffers = {
                { path, pathSize }
            },
        );
    }

    Result GetActiveVirtualAmiiboAreas(VirtualAmiiboAreaEntry* outAreaBuf, size_t outAreaSize, u32* outAreaCount) {
        return serviceDispatchOut(&g_EmuiiboService, 11, *outAreaCount,
            .buffer_attrs = {
                SfBufferAttr_HipcMapAlias | SfBufferAttr_Out
            },
            .buffers = {
                { outAreaBuf, outAreaSize }
            },
        );
    }

    Result GetActiveVirtualAmiiboCurrentArea(u32* outAccessId) {
        return serviceDispatchOut(&g_EmuiiboService, 12, *outAccessId);
    }

    Result SetActiveVirtualAmiiboCurrentArea(u32 accessId) {
        return serviceDispatchIn(&g_EmuiiboService, 13, accessId);
    }

    Result SetActiveVirtualAmiiboUuidInfo(VirtualAmiiboUuidInfo uuidInfo) {
        return serviceDispatchIn(&g_EmuiiboService, 14, uuidInfo);
    }

}  // namespace emu
