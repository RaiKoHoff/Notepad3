#pragma once

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <wchar.h>

#define IS_VALID_HANDLE(h) ((h) && (h) != INVALID_HANDLE_VALUE)
#define WM_FILECHANGEDNOTIFY (WM_USER + 2)
#define WM_FILEWATCHFAILED (WM_USER + 3)
#define ID_WATCHTIMER 1
#define ID_ATOMICSAVETIMER 2

typedef const wchar_t* HPATHL;
static struct { HWND hwndMain = reinterpret_cast<HWND>(1); } Globals;
static struct { LONG64 FileCheckInterval = 500; } FileWatching;

static bool Path_IsEmpty(HPATHL path) { return !path || !*path; }
static bool Path_IsNotEmpty(HPATHL path) { return !Path_IsEmpty(path); }
static LPCWSTR Path_Get(HPATHL path) { return path; }
static void Path_Release(HPATHL) {}
static LONG64 GetTicks_ms() { return (LONG64)GetTickCount64(); }
static bool IsAppClosing() { return false; }
static void CALLBACK WatchTimerProc(HWND, UINT, UINT_PTR, DWORD) {}

static bool TestGetAttributes(HPATHL, GET_FILEEX_INFO_LEVELS, LPVOID);
static HANDLE TestFindFirst(LPCWSTR, BOOL, DWORD);
static BOOL TestFindNext(HANDLE);
static BOOL TestPostMessage(HWND, UINT, WPARAM, LPARAM);
static UINT_PTR TestSetTimer(HWND, UINT_PTR, UINT, TIMERPROC);
static BOOL TestKillTimer(HWND, UINT_PTR);
static DWORD TestWaitForMultipleObjects(DWORD, const HANDLE*, BOOL, DWORD);

#define Path_GetFileAttributesEx TestGetAttributes
#define FindFirstChangeNotificationW TestFindFirst
#define FindNextChangeNotification TestFindNext
#define FindCloseChangeNotification CloseHandle
#define SetTimer TestSetTimer
#define KillTimer TestKillTimer
#define WaitForMultipleObjects TestWaitForMultipleObjects
#undef PostMessage
#define PostMessage TestPostMessage
