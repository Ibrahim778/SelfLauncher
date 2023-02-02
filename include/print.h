#ifndef PRINT_H
#define PRINT_H

#ifdef _DEBUG
#define print(...) sceClibPrintf(__VA_ARGS__)
#else
#define print(...) {(void)SCE_NULL;}
#endif

#endif