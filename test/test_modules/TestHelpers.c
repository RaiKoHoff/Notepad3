#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <wchar.h>
#include "Helpers.h"
#include "PathLib.h"

GLOBALS_T Globals;
SETTINGS_T Settings;
PATHS_T Paths;
HANDLE g_hndlScintilla;

#define UNEXPECTED() do { fprintf(stderr, "Unexpected application stub: %s\n", __func__); abort(); } while (0)
int timsort(void* base, size_t count, size_t width, int (*compare)(const void*, const void*))
{ (void)base; (void)count; (void)width; (void)compare; UNEXPECTED(); }
DWORD InfoBoxLastError(LPCWSTR message, DWORD error)
{ (void)message; (void)error; UNEXPECTED(); }
DWORD GetWindowsBuildNumber(LPDWORD major, LPDWORD minor)
{ (void)major; (void)minor; UNEXPECTED(); }
LRESULT WINAPI Scintilla_DirectFunction(HANDLE handle, UINT message, WPARAM wparam, LPARAM lparam)
{ (void)handle; (void)message; (void)wparam; (void)lparam; UNEXPECTED(); }

static int failures;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

static volatile LONG failThreadWait;
static jmp_buf failFastTarget;
static unsigned int failFastCode;

DWORD WINAPI TestWaitForSingleObject(HANDLE handle, DWORD milliseconds)
{
    if (InterlockedExchange(&failThreadWait, 0)) {
        SetLastError(ERROR_INVALID_HANDLE);
        return WAIT_FAILED;
    }
    return WaitForSingleObject(handle, milliseconds);
}

__declspec(noreturn) void TestFastFail(unsigned int code)
{
    failFastCode = code;
    longjmp(failFastTarget, 1);
}

typedef struct WorkerContext {
    BackgroundWorker* worker;
    HANDLE started;
    HANDLE delay;
    DWORD waitResult;
    bool pathValidAfterCancel;
} WorkerContext;

static unsigned int __stdcall WaitForCancellation(void* parameter)
{
    WorkerContext* context = (WorkerContext*)parameter;
    SetEvent(context->started);
    context->waitResult = WaitForSingleObject(context->worker->eventCancel, 5000);
    if (context->delay && WaitForSingleObject(context->delay, 10000) != WAIT_OBJECT_0) abort();
    LPCWSTR path = Path_Get(context->worker->hFilePath);
    context->pathValidAfterCancel = path && wcscmp(path, L"C:\\worker\\file.txt") == 0;
    return 0;
}

static unsigned int __stdcall UnexpectedSecondStart(void* parameter)
{
    (void)parameter;
    abort();
}

int main(void)
{
    Globals.hndlProcessHeap = GetProcessHeap();
    WCHAR path[128] = L"C:\\folder\\name.c:42:7";
    int line = 0;
    int column = 0;
    CHECK(SplitFilePathLineColNum(path, &line, &column));
    CHECK(wcscmp(path, L"C:\\folder\\name.c") == 0 && line == 42 && column == 7);
    CHECK(SUCCEEDED(StringCchCopyW(path, ARRAYSIZE(path), L"C:\\folder\\name.c(25)")));
    CHECK(SplitFilePathLineColNum(path, &line, &column));
    CHECK(wcscmp(path, L"C:\\folder\\name.c") == 0 && line == 25 && column == -1);
    CHECK(SUCCEEDED(StringCchCopyW(path, ARRAYSIZE(path), L"C:\\folder\\name.c:42+1")));
    CHECK(!SplitFilePathLineColNum(path, &line, &column));
    CHECK(wcscmp(path, L"C:\\folder\\name.c:42+1") == 0);

    WCHAR first[128];
    WCHAR rest[128];
    CHECK(ExtractFirstArgument(L" \"quoted file.txt\"  --next value", first, rest, ARRAYSIZE(first)));
    CHECK(wcscmp(first, L"quoted file.txt") == 0 && wcscmp(rest, L"--next value") == 0);
    CHECK(!ExtractFirstArgument(L"", first, rest, ARRAYSIZE(first)));

    WCHAR escaped[128];
    CHECK(SlashCtrlW(escaped, ARRAYSIZE(escaped), L"a\nb\tc") == 7);
    CHECK(wcscmp(escaped, L"a\\nb\\tc") == 0);
    CHECK(UnSlashCtrlW(escaped) == 5 && wcscmp(escaped, L"a\nb\tc") == 0);
    WCHAR text[32] = L"oneXXtwoXX";
    CHECK(StrCutIW(text, L"xx") == text && wcscmp(text, L"onetwo") == 0);

    LPWSTR replaced = StrReplaceAll(L"alpha beta alpha", L"alpha", L"x");
    CHECK(replaced && wcscmp(replaced, L"x beta x") == 0);
    FreeMem(replaced);
    replaced = StrReplaceAll(L"unchanged", L"missing", L"x");
    CHECK(replaced && wcscmp(replaced, L"unchanged") == 0);
    FreeMem(replaced);
    CHECK(StrReplaceAll(L"unchanged", L"", L"x") == NULL);

    HPATHL input = Path_Allocate(L"C:\\worker\\file.txt");
    BackgroundWorker worker = { 0 };
    WorkerContext context = { &worker, CreateEventW(NULL, TRUE, FALSE, NULL), NULL, WAIT_FAILED, false };
    CHECK(input && context.started);
    if (!input || !context.started) return 1;
    BackgroundWorker_Init(&worker, NULL, input);
    CHECK(BackgroundWorker_Continue(&worker));
    CHECK(worker.hFilePath && wcscmp(Path_Get(worker.hFilePath), Path_Get(input)) == 0);
    CHECK(worker.hFilePath != input);
    BackgroundWorker_Start(&worker, WaitForCancellation, &context);
    CHECK(WaitForSingleObject(context.started, 5000) == WAIT_OBJECT_0);
    HANDLE const firstThread = worker.workerThread;
    BackgroundWorker_Start(&worker, UnexpectedSecondStart, NULL);
    CHECK(worker.workerThread == firstThread);
    BackgroundWorker_Cancel(&worker);
    CHECK(context.waitResult == WAIT_OBJECT_0 && context.pathValidAfterCancel);
    CHECK(!BackgroundWorker_Continue(&worker));

    CHECK(ResetEvent(context.started));
    context.delay = CreateEventW(NULL, TRUE, FALSE, NULL);
    CHECK(context.delay != NULL);
    if (!context.delay) return 1;
    BackgroundWorker_Start(&worker, WaitForCancellation, &context);
    CHECK(WaitForSingleObject(context.started, 5000) == WAIT_OBJECT_0);
    HANDLE const thread = worker.workerThread;
    InterlockedExchange(&failThreadWait, 1);
    failFastCode = 0;
    if (setjmp(failFastTarget) == 0) {
        BackgroundWorker_Cancel(&worker);
        CHECK(false);
    } else {
        CHECK(failFastCode == FAST_FAIL_FATAL_APP_EXIT);
    }
    CHECK(SetEvent(context.delay));
    CHECK(worker.workerThread == thread);
    BackgroundWorker_Cancel(&worker);
    CloseHandle(context.delay);
    context.delay = NULL;

    CHECK(ResetEvent(context.started));
    context.waitResult = WAIT_FAILED;
    context.pathValidAfterCancel = false;
    BackgroundWorker_Start(&worker, WaitForCancellation, &context);
    CHECK(WaitForSingleObject(context.started, 5000) == WAIT_OBJECT_0);
    BackgroundWorker_Destroy(&worker);
    CHECK(context.waitResult == WAIT_OBJECT_0 && context.pathValidAfterCancel);
    CHECK(worker.hFilePath == NULL);
    CloseHandle(context.started);
    Path_Release(input);

    if (failures) return 1;
    puts("Helpers C-caller tests passed.");
    return 0;
}
