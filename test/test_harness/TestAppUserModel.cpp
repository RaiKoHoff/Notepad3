#define CINTERFACE
#include <windows.h>
#include <shobjidl.h>
#include <propkey.h>
#include <strsafe.h>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <string>
#include "..\..\src\TypeDefs.h"

static IPropertyStoreVtbl storeVtbl{};
static IPropertyStore store{ &storeVtbl };
static HRESULT storeResult = S_OK;
static HRESULT idResult = S_OK;
static HRESULT stubCommandResult = S_OK;
static int propertyCalls;
static int releases;
static int allocations;
static int pathReleases;
static WCHAR recordedCommand[256];
static WCHAR display[256];
static WCHAR appID[256];
static bool injectArgumentFailure;
static bool failNextNew;
static bool failPathAllocation;
static bool missingExecutable;
static int reportedErrors;
static DWORD reportedError;
static LPCWSTR reportedContext;
static bool reportedAfterRelease;
struct TestPath { WCHAR text[128]; };

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

bool StrIsEmpty(LPCWSTR text) { return !text || !*text; }
int StringCchCompareXI(LPCWSTR a, LPCWSTR b) { return CompareStringOrdinal(a, -1, b, -1, TRUE) - CSTR_EQUAL; }
HPATHL Path_Allocate(LPCWSTR) {
    if (failPathAllocation) { return nullptr; }
    ++allocations;
    return reinterpret_cast<HPATHL>(new TestPath{});
}
void Path_GetModuleFilePath(HPATHL path) {
    StringCchCopyW(reinterpret_cast<TestPath*>(path)->text, 128, L"C:\\Program Files\\Notepad3.exe");
    if (injectArgumentFailure) { failNextNew = true; }
}
LPCWSTR Path_Get(HPATHL path) { return missingExecutable ? nullptr : reinterpret_cast<TestPath*>(path)->text; }
void Path_Release(HPATHL path) { if (path) { ++pathReleases; delete reinterpret_cast<TestPath*>(path); } }
ULONG WINAPI ReleaseStore(IPropertyStore*) { ++releases; return 0; }
HRESULT TestGetPropertyStore(HWND, REFIID, void** output) {
    if (SUCCEEDED(storeResult)) { *output = &store; }
    return storeResult;
}
HRESULT _SetPropertyString(IPropertyStore*, REFPROPERTYKEY key, PCWSTR value) {
    ++propertyCalls;
    if (IsEqualPropertyKey(key, PKEY_AppUserModel_RelaunchCommand)) {
        StringCchCopyW(recordedCommand, _countof(recordedCommand), value ? value : L"");
        return stubCommandResult;
    } else if (IsEqualPropertyKey(key, PKEY_AppUserModel_RelaunchDisplayNameResource)) {
        StringCchCopyW(display, _countof(display), value ? value : L"");
    } else if (IsEqualPropertyKey(key, PKEY_AppUserModel_ID)) {
        StringCchCopyW(appID, _countof(appID), value ? value : L"");
        return idResult;
    }
    return S_OK;
}
DWORD InfoBoxLastError(LPCWSTR context, DWORD error) {
    ++reportedErrors;
    reportedError = error;
    reportedContext = context;
    reportedAfterRelease = releases > 0;
    return error;
}
#define SHGetPropertyStoreForWindow TestGetPropertyStore
#include "AppUserModel.inc"

static int failures;
#define CHECK(expr) do { if (!(expr)) { std::printf("Failed at line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)
int main()
{
    storeVtbl.Release = ReleaseStore;
    HWND const hwnd = reinterpret_cast<HWND>(1);
    CHECK(SetWindowAppUserModelID(hwnd, L"NP3.Test", L"Notepad3") == S_OK);
    CHECK(propertyCalls == 3 && releases == 1 && allocations == 1 && pathReleases == 1);
    CHECK(wcscmp(recordedCommand, L"\"C:\\Program Files\\Notepad3.exe\"") == 0);
    CHECK(wcscmp(display, L"Notepad3") == 0 && wcscmp(appID, L"NP3.Test") == 0);
    CHECK(SetWindowAppUserModelID(nullptr, L"NP3.Test", L"Notepad3") == S_OK);
    CHECK(SetWindowAppUserModelID(hwnd, L"", L"Notepad3") == S_OK);
    CHECK(SetWindowAppUserModelID(hwnd, L"(DEFAULT)", L"Notepad3") == S_OK);
    CHECK(propertyCalls == 3 && releases == 1 && allocations == 1);
    idResult = E_ACCESSDENIED;
    CHECK(SetWindowAppUserModelID(hwnd, L"NP3.Test", L"Notepad3") == E_ACCESSDENIED);
    CHECK(propertyCalls == 6 && releases == 2 && allocations == pathReleases);
    storeResult = E_FAIL;
    CHECK(SetWindowAppUserModelID(hwnd, L"NP3.Test", L"Notepad3") == E_FAIL);
    CHECK(propertyCalls == 6 && releases == 2 && allocations == pathReleases);
    propertyCalls = releases = allocations = pathReleases = 0;
    storeResult = idResult = S_OK;
    recordedCommand[0] = display[0] = appID[0] = L'\0';
    injectArgumentFailure = true;
    HRESULT const result = SetWindowAppUserModelID(hwnd, L"NP3.Test", L"Notepad3");
    injectArgumentFailure = false;
    CHECK(result == S_OK && propertyCalls == 2 && recordedCommand[0] == L'\0');
    CHECK(!failNextNew && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    CHECK(reportedContext && wcscmp(reportedContext, L"AppUserModelID") == 0 && reportedAfterRelease);
    CHECK(wcscmp(display, L"Notepad3") == 0 && wcscmp(appID, L"NP3.Test") == 0);
    CHECK(releases == 1 && allocations == 1 && pathReleases == 1);
    propertyCalls = releases = allocations = pathReleases = reportedErrors = 0;
    failPathAllocation = true;
    CHECK(SetWindowAppUserModelID(hwnd, L"NP3.Test", L"Notepad3") == S_OK);
    failPathAllocation = false;
    CHECK(propertyCalls == 2 && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    CHECK(releases == 1 && allocations == 0 && pathReleases == 0);
    propertyCalls = releases = allocations = pathReleases = reportedErrors = 0;
    missingExecutable = true;
    CHECK(SetWindowAppUserModelID(hwnd, L"NP3.Test", L"Notepad3") == S_OK);
    missingExecutable = false;
    CHECK(propertyCalls == 2 && reportedErrors == 1 && releases == 1 && allocations == pathReleases);
    propertyCalls = releases = allocations = pathReleases = reportedErrors = 0;
    stubCommandResult = E_OUTOFMEMORY;
    idResult = E_ACCESSDENIED;
    CHECK(SetWindowAppUserModelID(hwnd, L"NP3.Test", L"Notepad3") == E_ACCESSDENIED);
    CHECK(propertyCalls == 3 && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    CHECK(releases == 1 && allocations == pathReleases);
    propertyCalls = releases = allocations = pathReleases = reportedErrors = 0;
    stubCommandResult = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    idResult = S_OK;
    CHECK(SetWindowAppUserModelID(hwnd, L"NP3.Test", L"Notepad3") == S_OK);
    CHECK(propertyCalls == 3 && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    CHECK(releases == 1 && allocations == pathReleases);
    std::printf("AppUserModel checks: %d failures\n", failures);
    return failures ? 1 : 0;
}
