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
#include <intrin.h>

#ifdef __cplusplus
extern "C" {
#endif
DWORD WINAPI TestWaitForSingleObject(HANDLE handle, DWORD milliseconds);
__declspec(noreturn) void TestFastFail(unsigned int code);
#ifdef __cplusplus
}
#endif

#define WaitForSingleObject TestWaitForSingleObject
#define __fastfail TestFastFail
