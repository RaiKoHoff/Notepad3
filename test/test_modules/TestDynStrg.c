#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "DynStrg.h"
#include <shellapi.h>

static bool failAlloc;
static bool failReAlloc;
static int allocFailures;
static int reallocFailures;
static LPVOID trackedHandle;
static LPVOID trackedData;
static int handleFrees;
static int dataFrees;

LPVOID WINAPI TestHeapAlloc(HANDLE heap, DWORD flags, SIZE_T bytes)
{
    if (failAlloc) {
        failAlloc = false;
        ++allocFailures;
        return NULL;
    }
    return HeapAlloc(heap, flags, bytes);
}

LPVOID WINAPI TestHeapReAlloc(HANDLE heap, DWORD flags, LPVOID memory, SIZE_T bytes)
{
    if (failReAlloc) {
        failReAlloc = false;
        ++reallocFailures;
        return NULL;
    }
    return HeapReAlloc(heap, flags, memory, bytes);
}

BOOL WINAPI TestHeapFree(HANDLE heap, DWORD flags, LPVOID memory)
{
    if (memory == trackedHandle) ++handleFrees;
    if (memory == trackedData) ++dataFrees;
    return HeapFree(heap, flags, memory);
}

static int failures;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

static void CheckDestruction(LPCWSTR initial)
{
    HSTRINGW value = StrgCreate(initial);
    CHECK(value != NULL);
    if (!value) return;
    trackedHandle = value;
    trackedData = (LPVOID)StrgGet(value);
    handleFrees = dataFrees = 0;
    StrgDestroy(value);
    CHECK(handleFrees == 1);
    CHECK(dataFrees == (trackedData ? 1 : 0));
    trackedHandle = trackedData = NULL;
}

static void CheckFreeKeepsHandle(void)
{
    HSTRINGW value = StrgCreate(L"initial");
    CHECK(value != NULL);
    if (!value) return;
    trackedHandle = value;
    trackedData = (LPVOID)StrgGet(value);
    handleFrees = dataFrees = 0;
    StrgFree(value);
    CHECK(handleFrees == 0 && dataFrees == 1);
    CHECK(StrgReset(value, L"reused") == 1);
    CHECK(wcscmp(StrgGet(value), L"reused") == 0);
    trackedData = (LPVOID)StrgGet(value);
    dataFrees = 0;
    StrgDestroy(value);
    CHECK(handleFrees == 1 && dataFrees == 1);
    trackedHandle = trackedData = NULL;
}

static void CheckEscaping(LPCWSTR input)
{
    HSTRINGW escaped = EscapeStringForCmdLine(input);
    WCHAR command[256];
    int argc = 0;
    CHECK(escaped != NULL && StrgGet(escaped) != NULL);
    if (!escaped || !StrgGet(escaped)) return;
    CHECK(SUCCEEDED(StringCchPrintfW(command, ARRAYSIZE(command), L"runner \"%s\"", StrgGet(escaped))));
    LPWSTR* argv = CommandLineToArgvW(command, &argc);
    CHECK(argv != NULL && argc == 2);
    if (argv) {
        if (argc == 2) CHECK(wcscmp(argv[1], input) == 0);
        LocalFree(argv);
    }
    StrgDestroy(escaped);
}

int main(void)
{
    CheckDestruction(NULL);
    CheckDestruction(L"example");
    CheckFreeKeepsHandle();
    StrgDestroy(NULL);

    failAlloc = true;
    CHECK(StrgCreate(NULL) == NULL);
    CHECK(allocFailures == 1);
    CHECK(StrgReset(NULL, L"invalid") == -1);

    HSTRINGW text = StrgCreate(NULL);
    CHECK(text != NULL);
    if (!text) return 1;
    CHECK(StrgGet(text) == NULL);
    CHECK(StrgGetLength(text) == 0 && StrgGetAllocLength(text) == 0);
    failAlloc = true;
    CHECK(StrgReset(text, L"alpha") == 0);
    CHECK(allocFailures == 2 && StrgGet(text) == NULL);
    CHECK(StrgReset(text, L"alpha") == 1);
    CHECK(wcscmp(StrgGet(text), L"alpha") == 0 && StrgGetLength(text) == 5);

    LPCWSTR original = StrgGet(text);
    size_t originalCapacity = StrgGetAllocLength(text);
    failReAlloc = true;
    CHECK(StrgWriteAccessBuf(text, originalCapacity + 128) == original);
    CHECK(reallocFailures == 1 && StrgGetAllocLength(text) == originalCapacity);
    CHECK(wcscmp(StrgGet(text), L"alpha") == 0);
    LPWSTR buffer = StrgWriteAccessBuf(text, 128);
    CHECK(buffer != NULL && StrgGetAllocLength(text) > 128);
    if (buffer) {
        CHECK(SUCCEEDED(StringCchCopyW(buffer, StrgGetAllocLength(text), L"edited")));
        StrgSanitize(text);
        CHECK(StrgGetLength(text) == 6 && wcscmp(StrgGet(text), L"edited") == 0);
    }

    HSTRINGW copy = StrgCopy(text);
    CHECK(copy != NULL && wcscmp(StrgGet(copy), L"edited") == 0);
    StrgCat(text, L" text");
    CHECK(wcscmp(StrgGet(copy), L"edited") == 0);
    CHECK(StrgInsert(text, 6, L" new") == 15);
    CHECK(wcscmp(StrgGet(text), L"edited new text") == 0);
    CHECK(StrgDelete(text, 6, 4) == 15);
    CHECK(wcscmp(StrgGet(text), L"edited text") == 0);
    StrgSwap(text, copy);
    CHECK(wcscmp(StrgGet(text), L"edited") == 0);
    CHECK(wcscmp(StrgGet(copy), L"edited text") == 0);
    CHECK(StrgResetFromUTF8(text, "\xc3\xa9") > 0);
    char utf8[16] = "";
    CHECK(StrgGetAsUTF8(text, utf8, (int)sizeof(utf8)) == 3);
    CHECK(strcmp(utf8, "\xc3\xa9") == 0);
    StrgFormat(text, L"%s %d", L"item", 42);
    CHECK(wcscmp(StrgGet(text), L"item 42") == 0);
    StrgFormat(text, L"%8.3s|%6d", L"alphabet", 42);
    CHECK(wcscmp(StrgGet(text), L"     alp|    42") == 0);
    CheckEscaping(L"");
    CheckEscaping(L"space and \"quotes\"");
    CheckEscaping(L"C:\\folder\\");
    CheckEscaping(L"before\\\"after");
    StrgDestroy(copy);
    StrgDestroy(text);
    if (failures) return 1;
    puts("DynStrg C-caller tests passed.");
    return 0;
}
