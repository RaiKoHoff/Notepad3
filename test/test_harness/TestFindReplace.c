#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "TestFindReplaceSupport.h"

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

typedef struct SearchResult {
    DocPos expectedStart, expectedEnd, position, matchStart, matchEnd;
    int status;
} SearchResult;

static SearchResult results[2];
static int resultCount, searchCount, searchFlags, searchStatus, changeHistory;
static DocPos targetStart, targetEnd;
static char lastPattern[8192];

DocPos SciCall_GetTargetStart(void) { return targetStart; }
DocPos SciCall_GetTargetEnd(void) { return targetEnd; }
void SciCall_SetTargetRange(DocPos start, DocPos end) { targetStart = start; targetEnd = end; }
void SciCall_SetSearchFlags(int flags) { searchFlags = flags; }
void SciCall_SetStatus(int status) { searchStatus = status; }
int SciCall_GetStatus(void) { return searchStatus; }
DocPos SciCall_PositionAfter(DocPos pos) { return pos + 1; }
DocPos SciCall_PositionBefore(DocPos pos) { return pos - 1; }
bool Sci_IsPosValid(DocPos pos)
{ return pos == ((pos > 0) ? SciCall_PositionAfter(SciCall_PositionBefore(pos)) : pos); }
int SciCall_GetChangeHistory(void) { return changeHistory; }

DocPos SciCall_SearchInTarget(DocPos length, const char* text)
{
    CHECK(searchCount < resultCount);
    if (searchCount >= resultCount) return -1;
    SearchResult result = results[searchCount++];
    CHECK(targetStart == result.expectedStart && targetEnd == result.expectedEnd);
    CHECK(length == (DocPos)strlen(text));
    CHECK(SUCCEEDED(StringCchCopyA(lastPattern, ARRAYSIZE(lastPattern), text)));
    targetStart = result.matchStart;
    targetEnd = result.matchEnd;
    searchStatus = result.status;
    return result.position;
}

static void ResetSearch(void)
{
    resultCount = searchCount = searchFlags = 0;
    searchStatus = SC_STATUS_WARN_REGEX;
    targetStart = 33;
    targetEnd = 44;
    lastPattern[0] = '\0';
    Globals.FindReplaceMatchFoundState = FND_NOP;
}

static void SetResult(int index, DocPos start, DocPos end, DocPos position,
                      DocPos matchStart, DocPos matchEnd, int status)
{
    SearchResult value = { start, end, position, matchStart, matchEnd, status };
    results[index] = value;
    if (resultCount <= index) resultCount = index + 1;
}

static void CheckReplacement(const char* input, bool regex, int history, const char* expected, int expectedMessage)
{
    char text[128];
    int message = -1;
    CHECK(SUCCEEDED(StringCchCopyA(text, ARRAYSIZE(text), input)));
    changeHistory = history;
    TestTransformReplacement(text, regex, &message);
    CHECK(strcmp(text, expected) == 0 && message == expectedMessage);
}

int main(void)
{
    DocPos begin, end;
    Globals.hndlProcessHeap = GetProcessHeap();

    ResetSearch();
    begin = 0; end = 20;
    SetResult(0, 0, 20, 4, 4, 7, SC_STATUS_OK);
    CHECK(TestFindInTarget(L"\u00e9", SCFIND_MATCHCASE, &begin, &end, false, 1) == 4);
    CHECK(begin == 4 && end == 7 && searchCount == 1 && searchFlags == SCFIND_MATCHCASE);
    CHECK(strcmp(lastPattern, "\xc3\xa9") == 0 && searchStatus == SC_STATUS_OK);
    CHECK(targetStart == 33 && targetEnd == 44 && Globals.FindReplaceMatchFoundState == NXT_FND);

    ResetSearch();
    begin = 20; end = 0;
    SetResult(0, 20, 0, 4, 4, 7, SC_STATUS_OK);
    CHECK(TestFindInTarget(L"\u00e9", SCFIND_WHOLEWORD, &begin, &end, false, 2) == 4);
    CHECK(begin == 4 && end == 7 && searchFlags == SCFIND_WHOLEWORD);
    CHECK(targetStart == 33 && targetEnd == 44 && Globals.FindReplaceMatchFoundState == PRV_WRP_FND);

    ResetSearch();
    begin = 0; end = 20;
    SetResult(0, 0, 20, -1, 0, 20, SC_STATUS_WARN_REGEX);
    CHECK(TestFindInTarget(L"(", SCFIND_REGEXP, &begin, &end, false, 1) == -1);
    CHECK(begin == 0 && end == 20 && targetStart == 33 && targetEnd == 44);
    CHECK(searchStatus == SC_STATUS_WARN_REGEX && Globals.FindReplaceMatchFoundState == FND_NOP);

    ResetSearch();
    begin = 3; end = 10;
    SetResult(0, 3, 10, 3, 3, 3, SC_STATUS_OK);
    SetResult(1, 4, 10, 6, 6, 7, SC_STATUS_OK);
    CHECK(TestFindInTarget(L"zero", SCFIND_REGEXP, &begin, &end, true, 0) == 6);
    CHECK(begin == 6 && end == 7 && searchCount == 2 && targetStart == 33 && targetEnd == 44);

    ResetSearch();
    begin = 7; end = 0;
    SetResult(0, 7, 0, 7, 7, 7, SC_STATUS_OK);
    SetResult(1, 6, 0, 4, 4, 5, SC_STATUS_OK);
    CHECK(TestFindInTarget(L"zero", SCFIND_REGEXP, &begin, &end, true, 0) == 4);
    CHECK(begin == 4 && end == 5 && searchCount == 2 && targetStart == 33 && targetEnd == 44);

    ResetSearch();
    begin = 0; end = 10;
    SetResult(0, 0, 10, -1, 0, 10, SC_STATUS_OK);
    CHECK(TestFindInTarget(L"missing", 0, &begin, &end, false, 1) == -1);
    CHECK(begin == 0 && end == 10 && searchCount == 1 && Globals.FindReplaceMatchFoundState == NXT_NOT_FND);
    CHECK(targetStart == 33 && targetEnd == 44);

    ResetSearch();
    begin = 3; end = 4;
    SetResult(0, 3, 4, 3, 3, 3, SC_STATUS_OK);
    CHECK(TestFindInTarget(L"zero", SCFIND_REGEXP, &begin, &end, true, 0) == -1);
    CHECK(begin == 3 && end == 4 && searchCount == 1 && targetStart == 33 && targetEnd == 44);

    ResetSearch();
    begin = 0; end = 10;
    CHECK(TestFindInTarget(L"", 0, &begin, &end, false, 1) == -1);
    CHECK(searchCount == 0 && searchStatus == SC_STATUS_WARN_REGEX && targetStart == 33 && targetEnd == 44);

    ResetSearch();
    WCHAR longPattern[8200];
    for (size_t i = 0; i < ARRAYSIZE(longPattern) - 1; ++i) longPattern[i] = L'a';
    longPattern[ARRAYSIZE(longPattern) - 1] = L'\0';
    begin = 0; end = 10;
    CHECK(TestFindInTarget(longPattern, 0, &begin, &end, false, 1) == -1);
    CHECK(searchCount == 0 && searchStatus == SC_STATUS_OK && Globals.FindReplaceMatchFoundState == NXT_NOT_FND);
    CHECK(targetStart == 33 && targetEnd == 44);

    CheckReplacement("a\\nb", false, 0, "a\nb", SCI_REPLACETARGET);
    CheckReplacement("a\\tb", false, 1, "a\tb", SCI_REPLACETARGETMINIMAL);
    CheckReplacement("$1\\n", true, 0, "$1\\n", SCI_REPLACETARGETRE);
    CheckReplacement("\\0", true, 1, "\\0", SCI_REPLACETARGETRE);
    CheckReplacement("a\\011b", true, 0, "a\tb", SCI_REPLACETARGET);

    WCHAR wildcard[128] = L"a*b?c+(x)|\\*";
    EDITFINDREPLACE data = { 0 };
    data.fuFlags = SCFIND_MATCHCASE;
    TestEscapeWildcards(wildcard, ARRAYSIZE(wildcard), &data);
    CHECK(wcscmp(wildcard, L"a.*b.c\\+\\(x\\)\\|\\*") == 0);
    CHECK(data.fuFlags == (SCFIND_MATCHCASE | SCFIND_REGEXP));

    if (failures) return 1;
    puts("FindReplace C-caller tests passed.");
    return 0;
}
