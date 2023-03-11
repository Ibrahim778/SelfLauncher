// Literally just https://gist.github.com/SKGleba/588516f185167c8dc3840ba8e7ca1bd9
// Thanks Gleba :)

#include <kernel.h>
#include <string.h>
#include <taihen.h>

extern int sceAppMgrLaunchAppByPath(const char *name, const char *cmd, int cmdlen, int dynamic, void *opt, void *id);

char lpath[0x400]; // self path
char larg[0x100];  // self args
uint32_t largl;    // self args len

tai_hook_ref_t QafHookRef;
SceUID         QafHookID = SCE_UID_INVALID_UID;
SceInt32 SceQafMgrForDriver_7B14DC45_Patched()
{
    TAI_NEXT(SceQafMgrForDriver_7B14DC45_Patched, QafHookRef);
    return 1;
}

tai_hook_ref_t SysrootHookRef;
SceUID         SysrootHookID = SCE_UID_INVALID_UID;
SceInt32 SceSysrootForDriver_421EFC96_Patched()
{
    TAI_NEXT(SceSysrootForDriver_421EFC96_Patched, SysrootHookRef);
    return 0;
}

SceInt32 LaunchThread(SceSize args, void *argp)
{
    sceDebugPrintf("Launch thread\n");
    int opt[52 / 4];
    memset(opt, 0, sizeof(opt));
    opt[0] = sizeof(opt);

    int ret = sceAppMgrLaunchAppByPath(lpath, (largl) ? larg : 0, largl, 0, opt, NULL);

    sceDebugPrintf("launch %s(%s) |=>| ret: 0x%X\n", lpath, larg, ret);

    return sceKernelExitDeleteThread(0);
}

/*
  Launches a self located @path with @cmd of @cmdlen (syscall)
   - @cmd & @cmdlen are optional
   - returns 0 or thread id

  Example: SLKernelLaunchSelfWithArgs("ux0:data/self_launcher/self.self", "-livearea_off", sizeof("-livearea_off"));
*/
int SLKernelLaunchSelfWithArgs(uintptr_t path, uintptr_t cmd, uint32_t cmdlen)
{
    uint32_t state;
    ENTER_SYSCALL(state);

    largl = (cmdlen < 0x100) ? cmdlen : 0x100;
    
    sceKernelStrncpyFromUser(lpath, path, 0x400);
    if (cmd)
        sceKernelCopyFromUser(larg, cmd, largl);

    SceUID thid = sceKernelCreateThread("SLKernelLaunchThread", LaunchThread, 0x40, 0x1000, 0, 0, NULL);
    if (thid < 0)
    {
        sceDebugPrintf("[SLKernel Error] Could not create launch_thread 0x%X\n", thid);
        EXIT_SYSCALL(state);
        return thid;
    }

    sceKernelStartThread(thid, 0, NULL);
    EXIT_SYSCALL(state);
    return 0;
}

int module_start(SceSize args, void *argp)
{
    //Patches 'n' shit   
    SysrootHookID = taiHookFunctionImportForKernel(KERNEL_PID, &SysrootHookRef, "SceAppMgr", 0x2ED7F97A, 0x421EFC96, SceSysrootForDriver_421EFC96_Patched);
    sceDebugPrintf("SysrootHookID: 0x%X\n", SysrootHookID);
    QafHookID = taiHookFunctionImportForKernel(KERNEL_PID, &QafHookRef, "SceAppMgr", 0x4E29D3B6, 0x7B14DC45, SceQafMgrForDriver_7B14DC45_Patched);
    sceDebugPrintf("QafHookID: 0x%X\n", QafHookID);
 
    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp)
{
    taiHookReleaseForKernel(SysrootHookID, SysrootHookRef);
    taiHookReleaseForKernel(QafHookID, QafHookRef);
    return SCE_KERNEL_STOP_SUCCESS;
}