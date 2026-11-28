#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "Helpers.h"
#include "PathLib.h"
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>

DWORD_PTR WINAPI TestMRUGetFileInfo(LPCWSTR path, DWORD attributes, SHFILEINFOW* info, UINT size, UINT flags);
void FileMRULookupIcon(void* input, bool noFadeHidden, LVITEMW* result);
#ifdef __cplusplus
}
#endif

#define SHGetFileInfoW TestMRUGetFileInfo
