// Literally just https://gist.github.com/SKGleba/588516f185167c8dc3840ba8e7ca1bd9
// Thanks Gleba :)

#include <kernel.h>
#include <string.h>
#include <taihen.h>

#include "SLKernel.h"

extern int sceAppMgrLaunchAppByPath(const char *name, const char *cmd, int cmdlen, int dynamic, void *opt, void *id);

SLKernelLaunchParam gParam;

SceUID SysrootHookID = SCE_UID_INVALID_UID;
SceUID QafHookID = SCE_UID_INVALID_UID;

tai_hook_ref_t SysrootHookRef;
tai_hook_ref_t QafHookRef;

SceInt32 SceQafMgrForDriver_7B14DC45_Patched()
{
    TAI_NEXT(SceQafMgrForDriver_7B14DC45_Patched, QafHookRef);
    taiHookReleaseForKernel(QafHookID, QafHookRef);
    return 1;
}

SceInt32 SceSysrootForDriver_421EFC96_Patched()
{
    TAI_NEXT(SceSysrootForDriver_421EFC96_Patched, SysrootHookRef);
    taiHookReleaseForKernel(SysrootHookID, SysrootHookRef);
    return 0;
}

SceInt32 LaunchThread(SceSize args, void *argp)
{
    sceDebugPrintf("Launch thread\n");

    //Patches 'n' shit   
    SysrootHookID = taiHookFunctionImportForKernel(KERNEL_PID, &SysrootHookRef, "SceAppMgr", 0x2ED7F97A, 0x421EFC96, SceSysrootForDriver_421EFC96_Patched);
    sceDebugPrintf("SysrootHookID: 0x%X\n", SysrootHookID);
    QafHookID = taiHookFunctionImportForKernel(KERNEL_PID, &QafHookRef, "SceAppMgr", 0x4E29D3B6, 0x7B14DC45, SceQafMgrForDriver_7B14DC45_Patched);
    sceDebugPrintf("QafHookID: 0x%X\n", QafHookID);
 
    int opt[52 / 4];
    memset(opt, 0, sizeof(opt));
    opt[0] = sizeof(opt);

    int ret = sceAppMgrLaunchAppByPath(gParam.path, (gParam.argLength) ? gParam.args : SCE_NULL, gParam.argLength, 0, opt, NULL);

    sceDebugPrintf("launch %s(%s) |=>| ret: 0x%X\n", gParam.path, gParam.args, ret);

    return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
    SceUID fd = sceIoOpen(SLKernelParamPath, SCE_O_RDONLY, 0);
    if(fd < 0)
        return SCE_KERNEL_START_FAILED;

    sceIoRead(fd, &gParam, sizeof(SLKernelLaunchParam));
    sceIoClose(fd);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp)
{
    SceUID thid = sceKernelCreateThread("SLKernelLaunchThread", LaunchThread, 0x40, 0x2000, 0, 0, NULL);
    if (thid < 0)
    {
        sceDebugPrintf("[SLKernel Error] Could not create SLKernelLaunchThread 0x%X\n", thid);
        return thid;
    }

    int r = sceKernelStartThread(thid, 0, SCE_NULL);
    sceKernelWaitThreadEnd(thid, SCE_NULL, SCE_NULL);
    sceDebugPrintf("Launch Result: 0x%X\n", r);
    return SCE_KERNEL_STOP_SUCCESS;
}