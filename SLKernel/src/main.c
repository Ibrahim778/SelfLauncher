// Literally just https://gist.github.com/SKGleba/588516f185167c8dc3840ba8e7ca1bd9

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/sblaimgr.h>

#include <stdio.h>
#include <string.h>

#define DACR_OFF(stmt)                     \
    do                                     \
    {                                      \
        unsigned prev_dacr;                \
        __asm__ volatile(                  \
            "mrc p15, 0, %0, c3, c0, 0 \n" \
            : "=r"(prev_dacr));            \
        __asm__ volatile(                  \
            "mcr p15, 0, %0, c3, c0, 0 \n" \
            :                              \
            : "r"(0xFFFF0000));            \
        stmt;                              \
        __asm__ volatile(                  \
            "mcr p15, 0, %0, c3, c0, 0 \n" \
            :                              \
            : "r"(prev_dacr));             \
    } while (0)

static char lpath[0x400]; // self path
static char larg[0x100];  // self args
static uint32_t largl;    // self args len

int ksceAppMgrLaunchAppByPath(const char *name, const char *cmd, int cmdlen, int dynamic, void *opt, void *id);

static int launch_thread(SceSize args, void *argp)
{
    int opt[52 / 4];
    memset(opt, 0, sizeof(opt));
    opt[0] = sizeof(opt);

    int ret = ksceAppMgrLaunchAppByPath(lpath, (largl) ? larg : 0, largl, 0, opt, NULL);

    ksceDebugPrintf("launch %s(%s) |=>| ret: 0x%X\n", lpath, larg, ret);

    return ksceKernelExitDeleteThread(0);
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
    
    ksceKernelStrncpyUserToKernel(lpath, path, 0x400);
    if (cmd)
        ksceKernelMemcpyUserToKernel(larg, cmd, largl);

    SceUID thid = ksceKernelCreateThread("launch_thread", (SceKernelThreadEntry)launch_thread, 0x40, 0x1000, 0, 0, NULL);
    if (thid < 0)
    {
        EXIT_SYSCALL(state);
        return thid;
    }

    ksceKernelStartThread(thid, 0, NULL);
    return 0;
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize args, void *argp)
{
    if(ksceSblAimgrIsTool())
    {
        ksceDebugPrintf("DEVKIT Skipping SceSysmem patches!\n");
        return SCE_KERNEL_START_SUCCESS;
    }

    // patch thread watchdog and allowSelfArgs QA
    uintptr_t addr;
    int sysmem_id = ksceKernelSearchModuleByName("SceSysmem");
    module_get_offset(0x10005, sysmem_id, 0, 0, &addr); // either taiModuleUtils or tai_compat
    DACR_OFF(*(uint32_t *)(addr + 0x1f0e6) = 0xe0032001; *(uint32_t *)(addr + 0x205fe) = 0xe00b2000;);
    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp)
{
    return SCE_KERNEL_STOP_SUCCESS;
}