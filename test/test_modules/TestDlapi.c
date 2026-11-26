#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "Helpers.h"
#include "Dlapi.h"

SETTINGS_T Settings;
#define UNEXPECTED() do { fprintf(stderr, "Unexpected application stub: %s\n", __func__); abort(); } while (0)
void BackgroundWorker_Init(BackgroundWorker* worker, HWND hwnd, const HPATHL path)
{ (void)worker; (void)hwnd; (void)path; UNEXPECTED(); }
void BackgroundWorker_Start(BackgroundWorker* worker, _beginthreadex_proc_type routine, LPVOID parameter)
{ (void)worker; (void)routine; (void)parameter; UNEXPECTED(); }
void BackgroundWorker_Cancel(BackgroundWorker* worker) { (void)worker; UNEXPECTED(); }
void BackgroundWorker_Destroy(BackgroundWorker* worker) { (void)worker; UNEXPECTED(); }

static int failures;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

static void CheckShellItem(LPCWSTR path, bool directory)
{
    PIDLIST_ABSOLUTE absolute = NULL;
    LPCITEMIDLIST child = NULL;
    IShellFolder* parent = NULL;
    HRESULT hr = SHParseDisplayName(path, NULL, &absolute, 0, NULL);
    CHECK(SUCCEEDED(hr) && absolute);
    if (FAILED(hr) || !absolute) return;
    hr = SHBindToParent(absolute, &IID_IShellFolder, (void**)&parent, &child);
    CHECK(SUCCEEDED(hr) && parent && child);
    if (SUCCEEDED(hr) && parent && child) {
        WCHAR display[MAX_PATH] = L"";
        bool displayResult = IL_GetDisplayName(parent, child, SHGDN_FORPARSING, display, ARRAYSIZE(display));
        CHECK(!displayResult && wcscmp(display, path) == 0);

        DL_FILTER filter;
        DirList_CreateFilter(&filter, L"*.txt;*.exe", false);
        CHECK(filter.nCount == 2);
        CHECK(wcscmp(filter.pFilter[0], L"*.txt") == 0 && wcscmp(filter.pFilter[1], L"*.exe") == 0);
        CHECK(DirList_MatchFilter(parent, child, &filter));
        DirList_CreateFilter(&filter, L"*.txt;*.exe", true);
        CHECK(DirList_MatchFilter(parent, child, &filter) == directory);
        DirList_CreateFilter(&filter, L"*.txt", false);
        CHECK(DirList_MatchFilter(parent, child, &filter) == directory);
        DirList_CreateFilter(&filter, L"*.txt", true);
        CHECK(DirList_MatchFilter(parent, child, &filter));
        DirList_CreateFilter(&filter, L"*.*", false);
        CHECK(filter.nCount == 0 && DirList_MatchFilter(parent, child, &filter));
        DirList_CreateFilter(&filter, L"*.*", true);
        CHECK(filter.nCount == 0 && DirList_MatchFilter(parent, child, &filter) == directory);
        parent->lpVtbl->Release(parent);
    }
    CoTaskMemFree(absolute);
}

int main(void)
{
    unsigned char first[] = { 4, 0, 0x11, 0x22, 0, 0 };
    unsigned char second[] = { 4, 0, 0x33, 0x44, 0, 0 };
    CHECK(IL_GetSize(NULL) == 0);
    CHECK(IL_GetSize((LPCITEMIDLIST)first) == 4);
    CHECK(IL_Create(NULL, 0, NULL, 0) == NULL);
    LPITEMIDLIST copy = IL_Create(NULL, 0, (LPCITEMIDLIST)second, 0);
    CHECK(copy && IL_GetSize(copy) == 4);
    if (copy) {
        CHECK(memcmp(copy, second, sizeof(second)) == 0);
        CoTaskMemFree(copy);
    }
    LPITEMIDLIST combined = IL_Create((LPCITEMIDLIST)first, 0, (LPCITEMIDLIST)second, 0);
    CHECK(combined && IL_GetSize(combined) == 8);
    if (combined) {
        const unsigned char expected[] = { 4, 0, 0x11, 0x22, 4, 0, 0x33, 0x44, 0, 0 };
        CHECK(memcmp(combined, expected, sizeof(expected)) == 0);
        CoTaskMemFree(combined);
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    CHECK(SUCCEEDED(hr));
    if (SUCCEEDED(hr)) {
        WCHAR executable[MAX_PATH];
        DWORD length = GetModuleFileNameW(NULL, executable, ARRAYSIZE(executable));
        CHECK(length > 0 && length < ARRAYSIZE(executable));
        if (length > 0 && length < ARRAYSIZE(executable)) {
            CheckShellItem(executable, false);
            CHECK(PathRemoveFileSpecW(executable));
            CheckShellItem(executable, true);
        }
        CoUninitialize();
    }
    if (failures) return 1;
    puts("Dlapi C-caller tests passed.");
    return 0;
}
