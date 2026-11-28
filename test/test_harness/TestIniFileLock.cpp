#include <windows.h>
#include <strsafe.h>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <string>

static int failures;
#define CHECK(expr) do { if (!(expr)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

static bool failNextNew;
static DWORD openError;
static DWORD lockError;
static DWORD reportedError;
static WCHAR reportedMessage[256];
static int openCalls;
static int lockCalls;
static int closeCalls;
static int reports;
static bool expectWrite;
static bool handleOpen;
static OVERLAPPED testOverlapped{};
static HANDLE const file = reinterpret_cast<HANDLE>(1);
static WCHAR const iniPath[] = L"C:\\settings with spaces\\Notepad3.ini";

void* operator new(size_t size)
{
    if (failNextNew) {
        failNextNew = false;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        throw std::bad_alloc();
    }
    if (void* memory = std::malloc(size ? size : 1)) {
        return memory;
    }
    throw std::bad_alloc();
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, size_t) noexcept { std::free(memory); }

static HANDLE TestCreateFile(LPCWSTR path, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security,
                             DWORD disposition, DWORD flags, HANDLE templateFile)
{
    ++openCalls;
    CHECK(!handleOpen && wcscmp(path, iniPath) == 0);
    CHECK(access == (expectWrite ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ));
    CHECK(disposition == (expectWrite ? DWORD{OPEN_ALWAYS} : DWORD{OPEN_EXISTING}));
    CHECK(share == (FILE_SHARE_READ | FILE_SHARE_WRITE) && flags == FILE_ATTRIBUTE_NORMAL);
    CHECK(!security && !templateFile);
    if (openError) {
        SetLastError(openError);
        return INVALID_HANDLE_VALUE;
    }
    handleOpen = true;
    return file;
}

static BOOL TestLockFileEx(HANDLE handle, DWORD flags, DWORD reserved, DWORD low, DWORD high,
                           LPOVERLAPPED ov)
{
    ++lockCalls;
    CHECK(handleOpen && handle == file && ov == &testOverlapped);
    CHECK(flags == (expectWrite ? DWORD{LOCKFILE_EXCLUSIVE_LOCK} : 0UL));
    CHECK(!reserved && low == MAXDWORD && !high);
    if (lockError) {
        SetLastError(lockError);
        return FALSE;
    }
    return TRUE;
}

static BOOL TestCloseHandle(HANDLE handle)
{
    ++closeCalls;
    CHECK(handleOpen && handle == file);
    handleOpen = false;
    SetLastError(ERROR_INVALID_HANDLE);
    return TRUE;
}

static void InfoBoxLastError(LPCWSTR message, DWORD error)
{
    ++reports;
    CHECK(!handleOpen);
    reportedError = error;
    CHECK(SUCCEEDED(StringCchCopyW(reportedMessage, ARRAYSIZE(reportedMessage), message)));
}

static bool StrIsEmpty(LPCWSTR text) { return !text || !*text; }
#define IS_VALID_HANDLE(handle) ((handle) && (handle) != INVALID_HANDLE_VALUE)
#undef CreateFile
#define CreateFile TestCreateFile
#define LockFileEx TestLockFileEx
#define CloseHandle TestCloseHandle
#include "IniFileLock.inc"
#undef CloseHandle

static void CheckLock(bool write)
{
    expectWrite = write;
    auto const acquire = write ? AcquireWriteFileLock : AcquireReadFileLock;
    openCalls = lockCalls = closeCalls = reports = 0;
    openError = lockError = 0;
    CHECK(acquire(nullptr, testOverlapped) == INVALID_HANDLE_VALUE);
    CHECK(acquire(L"", testOverlapped) == INVALID_HANDLE_VALUE);
    CHECK(!openCalls && !lockCalls && !closeCalls && !reports);

    CHECK(acquire(iniPath, testOverlapped) == file);
    CHECK(openCalls == 1 && lockCalls == 1 && !closeCalls && !reports && handleOpen);
    TestCloseHandle(file);

    openError = ERROR_ACCESS_DENIED;
    CHECK(acquire(iniPath, testOverlapped) == INVALID_HANDLE_VALUE);
    CHECK(openCalls == 2 && lockCalls == 1 && closeCalls == 1 && reports == 1);
    CHECK(reportedError == ERROR_ACCESS_DENIED);
    CHECK(wcscmp(reportedMessage, write
          ? L"AcquireWriteFileLock(C:\\settings with spaces\\Notepad3.ini): INVALID FILE HANDLE!"
          : L"AcquireReadFileLock(C:\\settings with spaces\\Notepad3.ini): INVALID FILE HANDLE!") == 0);

    openError = 0;
    lockError = ERROR_LOCK_VIOLATION;
    CHECK(acquire(iniPath, testOverlapped) == INVALID_HANDLE_VALUE);
    CHECK(openCalls == 3 && lockCalls == 2 && closeCalls == 2 && reports == 2);
    CHECK(reportedError == ERROR_LOCK_VIOLATION);
    CHECK(wcscmp(reportedMessage, write
          ? L"AcquireWriteFileLock(C:\\settings with spaces\\Notepad3.ini): NO EXCLUSIVE LOCK ACQUIRED!"
          : L"AcquireReadFileLock(C:\\settings with spaces\\Notepad3.ini): NO READER LOCK ACQUIRED!") == 0);

    failNextNew = true;
    CHECK(acquire(iniPath, testOverlapped) == INVALID_HANDLE_VALUE);
    CHECK(!failNextNew && closeCalls == 3 && reports == 3 && !handleOpen);
    CHECK(reportedError == ERROR_LOCK_VIOLATION);
    CHECK(wcscmp(reportedMessage, write ? L"AcquireWriteFileLock" : L"AcquireReadFileLock") == 0);
}

int main()
{
    CheckLock(false);
    CheckLock(true);
    if (failures) return 1;
    std::puts("INI file lock tests passed.");
    return 0;
}
