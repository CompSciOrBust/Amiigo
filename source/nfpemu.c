#include "nfpemu.h"
#include <string.h>

bool nfpemuIsAccessible()
{
	SmServiceName nfpemuService = smEncodeName("nfp:emu");
    Handle tmph = 0;
    Result rc = smRegisterService(&tmph, nfpemuService, false, 1);
    if(R_FAILED(rc)) return true;
    smUnregisterService(nfpemuService);
    return false;
}

static Service g_nfpEmuSrv;
static u64 g_refCnt;

Result nfpemuInitialize()
{
    atomicIncrement64(&g_refCnt);
    if(serviceIsActive(&g_nfpEmuSrv)) return 0;
	if (nfpemuIsAccessible())
    return smGetService(&g_nfpEmuSrv, "nfp:emu");
	return 0;
}

void nfpemuExit()
{
    if(atomicDecrement64(&g_refCnt) == 0) serviceClose(&g_nfpEmuSrv);
}

Result nfpemuGetCurrentAmiibo(char *out)
{
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddRecvBuffer(&c, out, FS_MAX_PATH, BufferType_Normal);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 0;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result nfpemuSetCustomAmiibo(const char *path)
{
    IpcCommand c;
    ipcInitialize(&c);
    char cpath[FS_MAX_PATH] = {0};
    strcpy(cpath, path);
    ipcAddSendBuffer(&c, cpath, FS_MAX_PATH, BufferType_Normal);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result nfpemuHasCustomAmiibo(bool *out_has)
{
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 2;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
            bool has;
        } *resp = r.Raw;

        rc = resp->result;
        if(R_SUCCEEDED(rc) && out_has) *out_has = resp->has;
    }

    return rc;
}

Result nfpemuResetCustomAmiibo()
{
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 3;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result nfpemuSetEmulationOnForever()
{
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 4;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result nfpemuSetEmulationOnOnce()
{
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 5;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result nfpemuSetEmulationOff()
{
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 6;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result nfpemuMoveToNextAmiibo()
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 7;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result nfpemuGetStatus(NfpEmuEmulationStatus *out)
{
	*out = 3;
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 8;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
            u32 status;
        } *resp = r.Raw;

        rc = resp->result;
        if(R_SUCCEEDED(rc) && out) *out = (NfpEmuEmulationStatus)resp->status;
    }

    return rc;
}

Result nfpemuRefresh()
{
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 9;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
        } *resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result nfpemuGetVersion(NfpEmuVersion *out_ver)
{
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 10;
    Result rc = serviceIpcDispatch(&g_nfpEmuSrv);

    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
            NfpEmuVersion ver;
        } *resp = r.Raw;

        rc = resp->result;
        if(R_SUCCEEDED(rc) && out_ver) *out_ver = resp->ver;
    }

    return rc;
}