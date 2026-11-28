#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <wchar.h>
#include <shlwapi.h>
#include <wininet.h>
#include "Helpers.h"
#include "PathLib.h"

GLOBALS_T Globals;
SETTINGS_T Settings;
PATHS_T Paths;
HANDLE g_hndlScintilla;
void TestFailNextNew(void);
bool TestExtractSelection(WCHAR* output, size_t capacity);
HPATHL TestResolveSelectionForOpen(int* line, bool* isDir);

#define UNEXPECTED() do { fprintf(stderr, "Unexpected application stub: %s\n", __func__); abort(); } while (0)
int timsort(void* base, size_t count, size_t width, int (*compare)(const void*, const void*))
{ (void)base; (void)count; (void)width; (void)compare; UNEXPECTED(); }
static int reportedErrors;
static DWORD reportedError;
static SIZE_T failHeapSize;
LPVOID WINAPI TestHeapAlloc(HANDLE heap, DWORD flags, SIZE_T bytes)
{
    if (failHeapSize && bytes >= failHeapSize) {
        failHeapSize = 0;
        return NULL;
    }
    return HeapAlloc(heap, flags, bytes);
}
LPVOID WINAPI TestHeapReAlloc(HANDLE heap, DWORD flags, LPVOID memory, SIZE_T bytes)
{
    if (failHeapSize && bytes >= failHeapSize) {
        failHeapSize = 0;
        return NULL;
    }
    return HeapReAlloc(heap, flags, memory, bytes);
}
BOOL WINAPI TestHeapFree(HANDLE heap, DWORD flags, LPVOID memory)
{
    return HeapFree(heap, flags, memory);
}
DWORD InfoBoxLastError(LPCWSTR message, DWORD error)
{
    if (!message || (wcscmp(message, L"ResolveSelectionToPath") != 0 &&
                     wcscmp(message, L"ExtractSelectionOrTokenAtCaret") != 0)) UNEXPECTED();
    ++reportedErrors;
    reportedError = error;
    return error;
}
DWORD GetWindowsBuildNumber(LPDWORD major, LPDWORD minor)
{ (void)major; (void)minor; UNEXPECTED(); }
static const char* document;
static DocPos documentLength;
static DocPos selectionStart;
static DocPos selectionEnd;
static DocPos caret;
static bool failRange;
static int failConversionCall;
static int conversionCalls;
int WINAPI TestMultiByteToWideChar(UINT codePage, DWORD flags, LPCCH input, int inputLength,
                                   LPWSTR output, int outputLength)
{
    if (++conversionCalls == failConversionCall) {
        SetLastError(ERROR_NO_UNICODE_TRANSLATION);
        return 0;
    }
    return MultiByteToWideChar(codePage, flags, input, inputLength, output, outputLength);
}
LRESULT WINAPI Scintilla_DirectFunction(HANDLE handle, UINT message, WPARAM wparam, LPARAM lparam)
{
    (void)handle;
    switch (message) {
    case SCI_GETSELECTIONEMPTY: return selectionStart == selectionEnd;
    case SCI_GETSELECTIONSTART: return selectionStart;
    case SCI_GETSELECTIONEND: return selectionEnd;
    case SCI_GETCURRENTPOS: return caret;
    case SCI_LINEFROMPOSITION: return 0;
    case SCI_POSITIONFROMLINE: return 0;
    case SCI_GETLINEENDPOSITION: return documentLength;
    case SCI_GETCHARAT:
        if ((DocPos)wparam >= 0 && (DocPos)wparam < documentLength) return (unsigned char)document[wparam];
        UNEXPECTED();
    case SCI_GETRANGEPOINTER:
        if (failRange) return 0;
        if ((DocPos)wparam >= 0 && lparam >= 0 && (DocPos)wparam + lparam <= documentLength)
            return (LRESULT)(document + wparam);
        UNEXPECTED();
    default: UNEXPECTED();
    }
}

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

static void CheckExtraction(const char* input, DocPos length, DocPos start, DocPos end,
                            DocPos pos, LPCWSTR expected)
{
    document = input;
    documentLength = length;
    selectionStart = start;
    selectionEnd = end;
    caret = pos;
    WCHAR token[256];
    CHECK(TestExtractSelection(token, ARRAYSIZE(token)));
    CHECK(wcscmp(token, expected) == 0);
}

int main(void)
{
    Globals.hndlProcessHeap = GetProcessHeap();
    CheckExtraction("  '<some path.txt>'  \nignored", 29, 0, 29, 0, L"some path.txt");
    CheckExtraction("abc\tdef", 7, 0, 7, 0, L"abc");
    CheckExtraction("a\0b", 3, 0, 3, 0, L"a");
    CheckExtraction("a\xc3\xa4.txt", 7, 0, 7, 0, L"a\u00e4.txt");
    CheckExtraction("before file.c:42, after", 23, 0, 0, 10, L"file.c:42");
    CheckExtraction("[x](path with (x).txt)", 22, 0, 0, 6, L"path with (x).txt");
    CheckExtraction(" , ", 3, 0, 0, 1, L"");
    document = "long selection for allocation failure";
    documentLength = selectionEnd = (DocPos)strlen(document);
    selectionStart = 0;
    WCHAR extracted[256];
    TestFailNextNew();
    CHECK(!TestExtractSelection(extracted, ARRAYSIZE(extracted)));
    CHECK(extracted[0] == L'\0' && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    failRange = true;
    CHECK(!TestExtractSelection(extracted, ARRAYSIZE(extracted)));
    CHECK(extracted[0] == L'\0' && reportedErrors == 2 && reportedError == ERROR_INVALID_DATA);
    failRange = false;
    conversionCalls = 0;
    failConversionCall = 1;
    CHECK(!TestExtractSelection(extracted, ARRAYSIZE(extracted)));
    CHECK(extracted[0] == L'\0' && reportedErrors == 3 && reportedError == ERROR_NO_UNICODE_TRANSLATION);
    conversionCalls = 0;
    failConversionCall = 2;
    CHECK(!TestExtractSelection(extracted, ARRAYSIZE(extracted)));
    CHECK(extracted[0] == L'\0' && reportedErrors == 4 && reportedError == ERROR_NO_UNICODE_TRANSLATION);
    failConversionCall = 0;
    reportedErrors = 0;
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

    WCHAR exePath[MAX_PATH];
    CHECK(GetModuleFileNameW(NULL, exePath, ARRAYSIZE(exePath)) > 0);
    Paths.CurrentFile = Path_Allocate(exePath);
    Paths.WorkingDirectory = Path_Copy(Paths.CurrentFile);
    Path_RemoveFileSpec(Paths.WorkingDirectory);
    HPATHL resolved = Path_Allocate(NULL);
    char selectedPath[MAX_PATH * 4 + 8];
    CHECK(WideCharToMultiByte(CP_UTF8, 0, exePath, -1, selectedPath, ARRAYSIZE(selectedPath), NULL, NULL) > 0);
    CHECK(SUCCEEDED(StringCchCatA(selectedPath, ARRAYSIZE(selectedPath), ":42")));
    document = selectedPath;
    documentLength = selectionEnd = (DocPos)strlen(selectedPath);
    selectionStart = 0;
    int selectedLine = -1;
    bool selectedIsDir = true;
    HPATHL selected = TestResolveSelectionForOpen(&selectedLine, &selectedIsDir);
    CHECK(selected && selectedLine == 42 && !selectedIsDir);
    CHECK(Path_StrgComparePath(selected, Paths.CurrentFile, Paths.WorkingDirectory, true) == 0);
    Path_Release(selected);
    failRange = true;
    CHECK(!TestResolveSelectionForOpen(&selectedLine, &selectedIsDir));
    CHECK(selectedLine == -1 && reportedErrors == 1 && reportedError == ERROR_INVALID_DATA);
    failRange = false;
    reportedErrors = 0;
    failHeapSize = 1;
    CHECK(!TestResolveSelectionForOpen(&selectedLine, &selectedIsDir));
    CHECK(!failHeapSize && reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    reportedErrors = 0;
    bool isDir = true;
    CHECK(ResolveSelectionToPath(exePath, resolved, &isDir));
    CHECK(!isDir && Path_StrgComparePath(resolved, Paths.CurrentFile, Paths.WorkingDirectory, true) == 0);
    isDir = true;
    CHECK(ResolveSelectionToPath(Path_FindFileName(Paths.CurrentFile), resolved, &isDir));
    CHECK(!isDir && Path_StrgComparePath(resolved, Paths.CurrentFile, Paths.WorkingDirectory, true) == 0);
    WCHAR wrapped[MAX_PATH + 16];
    CHECK(SUCCEEDED(StringCchPrintfW(wrapped, ARRAYSIZE(wrapped), L" \t\"%s\"\t ", exePath)));
    CHECK(ResolveSelectionToPath(wrapped, resolved, NULL));
    CHECK(Path_StrgComparePath(resolved, Paths.CurrentFile, Paths.WorkingDirectory, true) == 0);
    CHECK(SUCCEEDED(StringCchPrintfW(wrapped, ARRAYSIZE(wrapped), L"\v'<%s>'\f", exePath)));
    CHECK(ResolveSelectionToPath(wrapped, resolved, NULL));
    CHECK(Path_StrgComparePath(resolved, Paths.CurrentFile, Paths.WorkingDirectory, true) == 0);
    WCHAR url[INTERNET_MAX_URL_LENGTH + 1];
    DWORD urlLength = ARRAYSIZE(url);
    CHECK(SUCCEEDED(UrlCreateFromPathW(exePath, url, &urlLength, 0)));
    isDir = true;
    CHECK(ResolveSelectionToPath(url, resolved, &isDir));
    CHECK(!isDir && Path_StrgComparePath(resolved, Paths.CurrentFile, Paths.WorkingDirectory, true) == 0);
    WCHAR longUrl[INTERNET_MAX_URL_LENGTH + 32];
    CHECK(SUCCEEDED(StringCchCopyW(longUrl, ARRAYSIZE(longUrl), L"file:///")));
    for (size_t i = 8; i < ARRAYSIZE(longUrl) - 1; ++i) longUrl[i] = L'a';
    longUrl[ARRAYSIZE(longUrl) - 1] = L'\0';
    WCHAR urlPath[INTERNET_MAX_URL_LENGTH + 1];
    DWORD urlPathLength = ARRAYSIZE(urlPath);
    CHECK(FAILED(PathCreateFromUrlW(longUrl, urlPath, &urlPathLength, 0)));
    HPATHL fallback = Path_Allocate(NULL);
    CHECK(!ResolveSelectionToPath(longUrl + 8, fallback, NULL));
    CHECK(!ResolveSelectionToPath(longUrl, resolved, NULL));
    CHECK(Path_StrgComparePath(resolved, fallback, Paths.WorkingDirectory, true) == 0);
    Path_Release(fallback);
    CHECK(SetEnvironmentVariableW(L"NP3_HELPERS_TEST_EXE_PATH", exePath));
    CHECK(ResolveSelectionToPath(L"%NP3_HELPERS_TEST_EXE_PATH%", resolved, NULL));
    CHECK(Path_StrgComparePath(resolved, Paths.CurrentFile, Paths.WorkingDirectory, true) == 0);
    CHECK(SetEnvironmentVariableW(L"NP3_HELPERS_TEST_EXE_PATH", NULL));
    HPATHL currentFile = Paths.CurrentFile;
    Paths.CurrentFile = NULL;
    CHECK(ResolveSelectionToPath(Path_FindFileName(currentFile), resolved, NULL));
    CHECK(Path_StrgComparePath(resolved, currentFile, Paths.WorkingDirectory, true) == 0);
    Paths.CurrentFile = currentFile;
    isDir = false;
    CHECK(ResolveSelectionToPath(Path_Get(Paths.WorkingDirectory), resolved, &isDir) && isDir);
    isDir = true;
    CHECK(!ResolveSelectionToPath(L"np3-test-file-that-does-not-exist", resolved, &isDir));
    CHECK(!isDir && Path_IsNotEmpty(resolved));
    isDir = true;
    CHECK(!ResolveSelectionToPath(L"", resolved, &isDir) && !isDir && Path_IsEmpty(resolved));
    isDir = true;
    CHECK(!ResolveSelectionToPath(exePath, NULL, &isDir) && !isDir);
    Path_Reset(resolved, L"previous output");
    isDir = true;
    TestFailNextNew();
    CHECK(!ResolveSelectionToPath(exePath, resolved, &isDir));
    CHECK(!isDir && Path_IsEmpty(resolved));
    CHECK(reportedErrors == 1 && reportedError == ERROR_OUTOFMEMORY);
    failHeapSize = (wcslen(exePath) + 1) * sizeof(WCHAR);
    CHECK(!ResolveSelectionToPath(exePath, resolved, &isDir));
    CHECK(!failHeapSize && !isDir && Path_IsEmpty(resolved));
    CHECK(reportedErrors == 2 && reportedError == ERROR_OUTOFMEMORY);
    Path_Release(resolved);
    Path_Release(Paths.WorkingDirectory);
    Path_Release(Paths.CurrentFile);
    Paths.WorkingDirectory = NULL;
    Paths.CurrentFile = NULL;

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
