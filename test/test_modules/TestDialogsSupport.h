#pragma once
#include <windows.h>
#include <shobjidl.h>
#include <stdlib.h>

#define MB_FILECHANGEDNOTIFY 0x0000000EL

#ifdef __cplusplus
extern "C" {
#endif
void* TestDialogAlloc(size_t size, DWORD flags);
#ifdef __cplusplus
}
#endif
#define AllocMem TestDialogAlloc
