#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
LPVOID WINAPI TestHeapAlloc(HANDLE heap, DWORD flags, SIZE_T bytes);
LPVOID WINAPI TestHeapReAlloc(HANDLE heap, DWORD flags, LPVOID memory, SIZE_T bytes);
BOOL WINAPI TestHeapFree(HANDLE heap, DWORD flags, LPVOID memory);
#ifdef __cplusplus
}
#endif

#define HeapAlloc TestHeapAlloc
#define HeapReAlloc TestHeapReAlloc
#define HeapFree TestHeapFree
