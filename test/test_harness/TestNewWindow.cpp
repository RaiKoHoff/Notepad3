#include <windows.h>
#include <shellapi.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include "..\..\src\TypeDefs.h"

#define COUNTOF(value) _countof(value)
struct TestPath { std::wstring text; };
PATHS_T Paths{};
SETTINGS2_T Settings2{};
FLAGS_T Flags{};
GLOBALS_T Globals{};
static int allocatedPaths;
static int releasedPaths;
static int launches;
static int saves;
static bool saveResult;
static bool launchResult = true;
static int closes;
static int registeredSnapshots;
static std::wstring launchedFile;
static std::wstring launchedArguments;
static std::wstring launchedDirectory;
static ULONG launchedMask;
static int launchedShow;
static bool injectArgumentFailure;
static bool failNextNew;
static bool failPathAllocation;
static HPATHL failOnPath;
static DWORD reportedError;
static int reportedErrors;
static LPCWSTR reportedContext;

void* operator new(size_t size) {
    if (failNextNew) {
        failNextNew = false;
        throw std::bad_alloc();
    }
    if (void* memory = std::malloc(size ? size : 1)) { return memory; }
    throw std::bad_alloc();
}
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, size_t) noexcept { std::free(memory); }

static TestPath* AsPath(HPATHL path) { return reinterpret_cast<TestPath*>(path); }
HPATHL Path_Allocate(LPCWSTR text) {
    if (failPathAllocation) { return nullptr; }
    ++allocatedPaths;
    return reinterpret_cast<HPATHL>(new TestPath{ text ? text : L"" });
}
void Path_Release(HPATHL path) {
    if (path) { ++releasedPaths; delete AsPath(path); }
}
LPCWSTR Path_Get(HPATHL path) {
    if (path && path == failOnPath) { failNextNew = true; }
    return path ? AsPath(path)->text.c_str() : nullptr;
}
bool Path_IsNotEmpty(HPATHL path) { return path && !AsPath(path)->text.empty(); }
void Path_GetModuleFilePath(HPATHL path) {
    AsPath(path)->text = L"C:\\Program Files\\Notepad3.exe";
    if (injectArgumentFailure) { failNextNew = true; }
}
bool FileSave(FileSaveFlags) { ++saves; return saveResult; }
WININFO GetMyWindowPlacement(HWND, MONITORINFO*, int, bool) {
    return { 10, 20, 800, 600, false, 110, 96 };
}
BOOL TestShellExecuteExW(SHELLEXECUTEINFOW* info) {
    failNextNew = false;
    ++launches;
    launchedFile = info->lpFile;
    launchedArguments = info->lpParameters ? info->lpParameters : L"";
    launchedDirectory = info->lpDirectory ? info->lpDirectory : L"";
    launchedMask = info->fMask;
    launchedShow = info->nShow;
    return launchResult ? TRUE : FALSE;
}
DWORD InfoBoxLastError(LPCWSTR context, DWORD error) {
    ++reportedErrors;
    reportedError = error;
    reportedContext = context;
    return error;
}
#define ShellExecuteExW TestShellExecuteExW
#include "NewWindow.inc"

void CloseApplication() { ++closes; }
void RunNewWindowReload() {
#include "NewWindowReload.inc"
}
void _RegisterDropSnapshot(HPATHL) { ++registeredSnapshots; }
void RunDroppedFile(HPATHL path, int& spawned) {
    struct DropEntry { HPATHL eff; } entry{ path };
    DropEntry* e = &entry;
    HWND hwnd = nullptr;
    bool sameFile = false;
    WININFO wi{ 10, 20, 800, 600, false, 110, 96 };
#include "NewWindowDrop.inc"
}

static int failures;
#define CHECK(expr) do { if (!(expr)) { std::printf("Failed at line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

int main()
{
    TestPath ini{ L"C:\\Settings Folder\\Notepad3.ini" };
    TestPath work{ L"C:\\Work" };
    TestPath file{ L"C:\\Notes\\some file.txt" };
    Paths.IniFile = reinterpret_cast<HPATHL>(&ini);
    Paths.WorkingDirectory = reinterpret_cast<HPATHL>(&work);
    HPATHL const filePath = reinterpret_cast<HPATHL>(&file);
    Paths.CurrentFile = filePath;
    StringCchCopyW(Settings2.AppUserModelID, COUNTOF(Settings2.AppUserModelID), L"NP3.Test");
    saveResult = true;

    CHECK(DialogNewWindow(nullptr, false, filePath, nullptr));
    CHECK(launches == 1 && saves == 0);
    CHECK(launchedFile == L"C:\\Program Files\\Notepad3.exe");
    CHECK(launchedArguments == L"-appid=\"NP3.Test\" -sysmru=\"0\" -f \"C:\\Settings Folder\\Notepad3.ini\" -n -pos 10,20,800,600,96,0,110 \"C:\\Notes\\some file.txt\"");
    CHECK(launchedDirectory == L"C:\\Work" && launchedMask == (SEE_MASK_NOASYNC | SEE_MASK_NOZONECHECKS)
        && launchedShow == SW_SHOWNORMAL);
    CHECK(allocatedPaths == releasedPaths);

    launches = saves = 0;
    Flags.bStickyWindowPosition = true;
    ini.text.clear();
    injectArgumentFailure = true;
    CHECK(!DialogNewWindow(nullptr, false, nullptr, nullptr));
    injectArgumentFailure = false;
    CHECK(launches == 0 && !failNextNew && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    CHECK(reportedContext && wcscmp(reportedContext, L"DialogNewWindow") == 0);
    CHECK(allocatedPaths == releasedPaths);

    launches = reportedErrors = 0;
    failPathAllocation = true;
    CHECK(!DialogNewWindow(nullptr, false, nullptr, nullptr));
    failPathAllocation = false;
    CHECK(launches == 0 && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    CHECK(allocatedPaths == releasedPaths);

    launches = reportedErrors = 0;
    file.text = L"C:\\Notes\\a-long-filename-that-requires-reallocation.txt";
    failOnPath = filePath;
    CHECK(!DialogNewWindow(nullptr, false, filePath, nullptr));
    failOnPath = nullptr;
    CHECK(launches == 0 && !failNextNew && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    CHECK(allocatedPaths == releasedPaths);

    launches = reportedErrors = 0;
    ini.text = L"\"already quoted.ini\"";
    file.text = L"\"already quoted.txt\"";
    CHECK(DialogNewWindow(nullptr, false, filePath, nullptr));
    CHECK(launches == 1 && launchedArguments ==
        L"-appid=\"NP3.Test\" -sysmru=\"0\" -f \"already quoted.ini\" -n \"already quoted.txt\"");
    CHECK(allocatedPaths == releasedPaths);

    launches = saves = 0;
    Flags.bSingleFileInstance = true;
    Flags.bStickyWindowPosition = true;
    Flags.ShellUseSystemMRU = true;
    ini.text.clear();
    CHECK(DialogNewWindow(nullptr, true, nullptr, nullptr));
    CHECK(launches == 1 && saves == 1);
    CHECK(launchedArguments == L"-appid=\"NP3.Test\" -sysmru=\"1\" -f0 -ns");
    CHECK(allocatedPaths == releasedPaths);

    launches = saves = 0;
    saveResult = false;
    CHECK(!DialogNewWindow(nullptr, true, filePath, nullptr));
    CHECK(launches == 0 && saves == 1 && allocatedPaths == releasedPaths);

    launches = saves = 0;
    Flags.bStickyWindowPosition = false;
    WININFO position{ -1, -2, 100, 200, true, 80, 144 };
    CHECK(DialogNewWindow(nullptr, false, nullptr, &position));
    CHECK(launches == 1 && launchedArguments ==
        L"-appid=\"NP3.Test\" -sysmru=\"1\" -f0 -ns -pos -1,-2,100,200,144,1,80");
    CHECK(allocatedPaths == releasedPaths);

    launches = saves = closes = reportedErrors = 0;
    saveResult = true;
    Flags.bStickyWindowPosition = true;
    injectArgumentFailure = true;
    RunNewWindowReload();
    injectArgumentFailure = false;
    CHECK(launches == 0 && closes == 0 && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    CHECK(allocatedPaths == releasedPaths);

    launches = saves = closes = 0;
    saveResult = false;
    RunNewWindowReload();
    CHECK(launches == 0 && closes == 0 && saves == 1);
    CHECK(allocatedPaths == releasedPaths);

    launches = saves = closes = 0;
    saveResult = true;
    launchResult = false;
    RunNewWindowReload();
    CHECK(launches == 1 && closes == 0 && saves == 1);
    CHECK(allocatedPaths == releasedPaths);

    launches = saves = closes = 0;
    launchResult = true;
    RunNewWindowReload();
    CHECK(launches == 1 && closes == 1 && saves == 1);
    CHECK(allocatedPaths == releasedPaths);

    launches = registeredSnapshots = 0;
    int spawned = 0;
    launchResult = false;
    RunDroppedFile(filePath, spawned);
    CHECK(launches == 1 && spawned == 0 && registeredSnapshots == 1);
    CHECK(allocatedPaths == releasedPaths);
    launchResult = true;
    RunDroppedFile(filePath, spawned);
    CHECK(launches == 2 && spawned == 1 && registeredSnapshots == 1);
    CHECK(allocatedPaths == releasedPaths);

    std::printf("New window checks: %d failures\n", failures);
    return failures ? 1 : 0;
}
