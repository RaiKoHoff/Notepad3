#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "Helpers.h"
#include "PathLib.h"
#include "Dlapi.h"

SETTINGS_T Settings;
static int failures;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)
#define REQUIRE(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ExitProcess(1); } } while (0)
static volatile LONG pidlIconCalls;
static volatile LONG invalidPidlFlags;
static volatile LONG failIconPostThread;
static volatile LONG failedIconPosts;

DWORD_PTR WINAPI TestDlapiGetFileInfo(LPCWSTR path, DWORD attributes, SHFILEINFOW* info, UINT size, UINT flags)
{
    if (flags & SHGFI_PIDL) {
        if (flags & SHGFI_USEFILEATTRIBUTES) InterlockedExchange(&invalidPidlFlags, TRUE);
        InterlockedIncrement(&pidlIconCalls);
    }
    return SHGetFileInfoW(path, attributes, info, size, flags);
}

HRESULT WINAPI TestDlapiBindToParent(PCIDLIST_ABSOLUTE pidl, REFIID iid, void** parent, PCUITEMID_CHILD* child)
{
    (void)pidl;
    (void)iid;
    *parent = NULL;
    *child = NULL;
    return E_NOINTERFACE;
}

BOOL WINAPI TestDlapiPostMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG const thread = (LONG)GetCurrentThreadId();
    if (InterlockedCompareExchange(&failIconPostThread, 0, thread) == thread) {
        InterlockedIncrement(&failedIconPosts);
        return FALSE;
    }
    return PostMessageW(hwnd, message, wParam, lParam);
}

typedef struct TestDirListData {
    UINT cbidl;
    LPITEMIDLIST pidl;
    LPSHELLFOLDER folder;
} TestDirListData;

typedef struct IconInput {
    HANDLE entered;
    HANDLE proceed;
    HANDLE released;
    int icon;
} IconInput;

static void LookupTestIcon(void* input, bool noFadeHidden, LVITEMW* result)
{
    IconInput* job = (IconInput*)input;
    (void)noFadeHidden;
    if (job->entered) SetEvent(job->entered);
    if (job->proceed && WaitForSingleObject(job->proceed, 5000) != WAIT_OBJECT_0) return;
    result->mask = LVIF_IMAGE;
    result->iImage = job->icon;
}

static void LookupIconWithFailedPost(void* input, bool noFadeHidden, LVITEMW* result)
{
    LookupTestIcon(input, noFadeHidden, result);
    InterlockedExchange(&failIconPostThread, (LONG)GetCurrentThreadId());
}

static void ReleaseTestIcon(void* input)
{
    IconInput* job = (IconInput*)input;
    if (job->released) SetEvent(job->released);
}

static void PumpPostedMessages(void)
{
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);
}

static int IconAt(HWND list, int index)
{
    LVITEMW item = { 0 };
    item.mask = LVIF_IMAGE;
    item.iItem = index;
    return ListView_GetItem(list, &item) ? item.iImage : -1;
}

static LRESULT CALLBACK CancelWhileApplying(HWND list, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR data)
{
    (void)id;
    if (msg == LVM_SETITEMW) {
        *(bool*)data = true;
        ListIconTask_Cancel(list);
    }
    return DefSubclassProc(list, msg, wParam, lParam);
}

static void CheckAsyncIcons(HWND list)
{
    LVITEMW item = { 0 };
    item.mask = LVIF_IMAGE;
    item.iImage = 4;
    CHECK(ListView_InsertItem(list, &item) == 0);

    IconInput only = { NULL, NULL, NULL, 8 };
    ListIconTask* task = ListIconTask_Create(list, true, LookupIconWithFailedPost, ReleaseTestIcon);
    REQUIRE(task);
    ListIconTask_SetInput(task, 0, &only);
    ListIconTask_Start(task);
    ULONGLONG deadline = GetTickCount64() + 5000;
    do {
        PumpPostedMessages();
        if (IconAt(list, 0) == 8) break;
        MsgWaitForMultipleObjects(0, NULL, FALSE, 50, QS_ALLINPUT);
    } while (GetTickCount64() < deadline);
    CHECK(IconAt(list, 0) == 8 && InterlockedCompareExchange(&failedIconPosts, 0, 0) == 1);
    CHECK(ListView_SetItem(list, &item));

    item.iItem = 1;
    CHECK(ListView_InsertItem(list, &item) == 1);
    IconInput early = { NULL, NULL, NULL, 9 };
    IconInput slow = { CreateEventW(NULL, TRUE, FALSE, NULL), CreateEventW(NULL, TRUE, FALSE, NULL),
                       CreateEventW(NULL, TRUE, FALSE, NULL), 10 };
    IconInput fresh = { NULL, NULL, CreateEventW(NULL, TRUE, FALSE, NULL), 11 };
    task = ListIconTask_Create(list, true, LookupTestIcon, ReleaseTestIcon);
    REQUIRE(task && slow.entered && slow.proceed && slow.released && fresh.released);
    ListIconTask_SetInput(task, 0, &early);
    ListIconTask_SetInput(task, 1, &slow);
    ListIconTask_Start(task);
    REQUIRE(WaitForSingleObject(slow.entered, 5000) == WAIT_OBJECT_0);
    ListIconTask_Cancel(list);
    CHECK(WaitForSingleObject(slow.released, 0) == WAIT_TIMEOUT);
    task = ListIconTask_Create(list, true, LookupTestIcon, ReleaseTestIcon);
    REQUIRE(task);
    ListIconTask_SetInput(task, 1, &fresh);
    ListIconTask_Start(task);
    deadline = GetTickCount64() + 5000;
    do {
        PumpPostedMessages();
        if (IconAt(list, 1) == 11) break;
        MsgWaitForMultipleObjects(0, NULL, FALSE, 50, QS_ALLINPUT);
    } while (GetTickCount64() < deadline);
    CHECK(IconAt(list, 0) == 4 && IconAt(list, 1) == 11);
    ListIconTask_Cancel(list);
    REQUIRE(WaitForSingleObject(fresh.released, 5000) == WAIT_OBJECT_0);
    CloseHandle(fresh.released);
    SetEvent(slow.proceed);
    REQUIRE(WaitForSingleObject(slow.released, 5000) == WAIT_OBJECT_0);

    bool reentered = false;
    REQUIRE(SetWindowSubclass(list, CancelWhileApplying, 0, (DWORD_PTR)&reentered));
    task = ListIconTask_Create(list, true, LookupTestIcon, ReleaseTestIcon);
    REQUIRE(task);
    ListIconTask_SetInput(task, 0, &early);
    ListIconTask_Start(task);
    deadline = GetTickCount64() + 5000;
    while (!reentered && GetTickCount64() < deadline) {
        MsgWaitForMultipleObjects(0, NULL, FALSE, 50, QS_ALLINPUT);
        PumpPostedMessages();
    }
    CHECK(reentered && IconAt(list, 0) == 9);
    CHECK(RemoveWindowSubclass(list, CancelWhileApplying, 0));

    ResetEvent(slow.entered);
    ResetEvent(slow.proceed);
    ResetEvent(slow.released);
    task = ListIconTask_Create(list, true, LookupTestIcon, ReleaseTestIcon);
    REQUIRE(task);
    ListIconTask_SetInput(task, 1, &slow);
    ListIconTask_Start(task);
    REQUIRE(WaitForSingleObject(slow.entered, 5000) == WAIT_OBJECT_0);
    CHECK(DestroyWindow(list));
    CHECK(WaitForSingleObject(slow.released, 0) == WAIT_TIMEOUT);
    SetEvent(slow.proceed);
    REQUIRE(WaitForSingleObject(slow.released, 5000) == WAIT_OBJECT_0);
    CloseHandle(slow.entered);
    CloseHandle(slow.proceed);
    CloseHandle(slow.released);
}

static LRESULT CALLBACK TestWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_NOTIFY) {
        NMHDR* header = (NMHDR*)lparam;
        if (GetPropW(header->hwndFrom, L"DirListData")) {
            if (header->code == LVN_DELETEITEM) DirList_DeleteItem(header->hwndFrom, lparam);
            if (header->code == LVN_GETDISPINFO) DirList_GetDispInfo(header->hwndFrom, lparam, false);
        }
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

static void CheckDirectoryPidl(LPCWSTR directory)
{
    INITCOMMONCONTROLSEX controls = { sizeof(controls), ICC_LISTVIEW_CLASSES };
    CHECK(InitCommonControlsEx(&controls));
    WNDCLASSW cls = { 0 };
    cls.lpfnWndProc = TestWindowProc;
    cls.hInstance = GetModuleHandleW(NULL);
    cls.lpszClassName = L"Notepad3DirListTest";
    CHECK(RegisterClassW(&cls) != 0);
    HWND parent = CreateWindowExW(0, cls.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, cls.hInstance, NULL);
    CHECK(parent != NULL);
    if (!parent) return;
    HWND list = CreateWindowExW(0, WC_LISTVIEWW, L"", WS_CHILD | LVS_REPORT, 0, 0, 100, 100, parent, NULL, cls.hInstance, NULL);
    CHECK(list != NULL);
    if (list) {
        HPATHL path = Path_Allocate(directory);
        CHECK(path != NULL);
        if (path) {
            CHECK(DirList_Init(list, L"", path));
            CHECK(DirList_Fill(list, directory, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, L"*.*", false, false, 0, false) >= 0);
            TestDirListData* data = (TestDirListData*)GetPropW(list, L"DirListData");
            CHECK(data && data->folder && data->pidl && data->cbidl == IL_GetSize(data->pidl));
            if (data && data->pidl) {
                WCHAR actual[MAX_PATH];
                CHECK(SHGetPathFromIDListW(data->pidl, actual));
                CHECK(_wcsicmp(actual, directory) == 0);
            }
            CHECK(ListView_GetItemCount(list) > 0);
            DirList_StartIconThread(list);
            ULONGLONG deadline = GetTickCount64() + 5000;
            while (!InterlockedCompareExchange(&pidlIconCalls, 0, 0) && GetTickCount64() < deadline) {
                MsgWaitForMultipleObjects(0, NULL, FALSE, 50, QS_ALLINPUT);
                PumpPostedMessages();
            }
            CHECK(InterlockedCompareExchange(&pidlIconCalls, 0, 0) > 0);
            CHECK(!InterlockedCompareExchange(&invalidPidlFlags, 0, 0));
            ListView_DeleteAllItems(list);
            DirList_Destroy(list);
            Path_Release(path);
        }
        CheckAsyncIcons(list);
    }
    DestroyWindow(parent);
    UnregisterClassW(cls.lpszClassName, cls.hInstance);
}

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
            CheckDirectoryPidl(executable);
        }
        CoUninitialize();
    }
    if (failures) return 1;
    puts("Dlapi C-caller tests passed.");
    return 0;
}
