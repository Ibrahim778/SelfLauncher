#ifndef _SL_KERNEL_H_
#define _SL_KERNEL_H_

#include <kernel.h>

SCE_CDECL_BEGIN

typedef struct {
    //  NULL terminated path
    char path[0x400];
    // Character string of length argLength
    char args[0x100];
    // Length of arguments
    unsigned int argLength;
} SLKernelLaunchParam;

#define SLKernelParamPath "ur0:data/sl_params"

SCE_CDECL_END

#endif