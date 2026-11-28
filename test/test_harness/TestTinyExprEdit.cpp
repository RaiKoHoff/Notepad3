#include "TypeDefs.h"
#include "tinyexprcpp/np3tinyexpr.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <strsafe.h>

#define COUNTOF(a) ARRAYSIZE(a)
#define CONSTSTRGLEN(s) (sizeof(s) - 1)
#define IDS_MUI_SELRECTORMULTI 1
#define Encoding_SciCP CP_UTF8
#define UndoTransActionBegin() { LONG token = 0; __try { token = BeginUndoActionSelection();
#define EndUndoTransAction() } __finally { EndUndoActionSelection(token); } }

static std::string document;
static DocPos selectionStart, selectionEnd, currentPos, anchorPos, targetPos;
static bool multiSelection, failReplace;
static int beginCount, endCount, warningCount;

static LONG BeginUndoActionSelection() { ++beginCount; return 0; }
static void EndUndoActionSelection(LONG) { ++endCount; }
static bool Sci_IsMultiOrRectangleSelection() { return multiSelection; }
static void InfoBoxLng(unsigned, void*, int) { ++warningCount; }
static bool SciCall_IsSelectionEmpty() { return selectionStart == selectionEnd; }
static void SciCall_SelectAll() { selectionStart = 0; selectionEnd = static_cast<DocPos>(document.size()); }
static DocPos SciCall_GetCurrentPos() { return currentPos; }
static DocPos SciCall_GetAnchor() { return anchorPos; }
static DocPos SciCall_GetSelectionStart() { return selectionStart; }
static DocPos SciCall_GetSelectionEnd() { return selectionEnd; }
static DocLn SciCall_LineFromPosition(DocPos pos)
{
    return static_cast<DocLn>(std::count(document.begin(), document.begin() + static_cast<size_t>(pos), '\n'));
}
static DocPos SciCall_PositionFromLine(DocLn line)
{
    size_t pos = 0;
    for (DocLn i = 0; i < line; ++i) {
        pos = document.find('\n', pos);
        if (pos == std::string::npos) { return static_cast<DocPos>(document.size()); }
        ++pos;
    }
    return static_cast<DocPos>(pos);
}
static DocPos SciCall_GetLineEndPosition(DocLn line)
{
    size_t pos = document.find('\n', static_cast<size_t>(SciCall_PositionFromLine(line)));
    return static_cast<DocPos>(pos == std::string::npos ? document.size() : pos);
}
static void SciCall_SetTargetRange(DocPos start, DocPos) { targetPos = start; }
static void SciCall_ReplaceTarget(DocPos, const char* text)
{
    if (failReplace) { RaiseException(0xE0000001, 0, 0, nullptr); }
    document.insert(static_cast<size_t>(targetPos), text);
}
static void EditSetSelectionEx(DocPos anchor, DocPos current, DocPos, DocPos)
{
    anchorPos = anchor;
    currentPos = current;
}
static bool StrIsNotEmpty(const WCHAR* s) { return s && *s; }
static bool StrIsEmptyA(const char* s) { return !s || !*s; }
static DocLn max_ln(DocLn a, DocLn b) { return (std::max)(a, b); }
static DocLn d2lnc(double d) { return std::isnan(d) ? 0 : static_cast<DocLn>(std::llround(d)); }
static size_t StringCchLenA(const char* s, size_t) { return std::strlen(s); }
static int WideCharToMultiByteEx(UINT cp, DWORD flags, const WCHAR* src, int len, char* dst, int size, const char* def, BOOL* used)
{
    return WideCharToMultiByte(cp, flags, src, len, dst, size, def, used);
}

#include "EditModifyLines.inc"

#define CHECK(condition) do { if (!(condition)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #condition); return 1; } } while (0)

static bool TryEditModifyLines(PENCLOSESELDATA patterns)
{
    __try {
        EditModifyLines(patterns);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return true;
    }
    return false;
}

int main()
{
    ENCLOSESELDATA patterns{};
    StringCchCopyW(patterns.pwsz1, COUNTOF(patterns.pwsz1), L"${N*10}:");
    StringCchCopyW(patterns.pwsz2, COUNTOF(patterns.pwsz2), L"${L+I}");
    document = "a\nb\nc";
    selectionStart = anchorPos = 0;
    selectionEnd = currentPos = static_cast<DocPos>(document.size());
    EditModifyLines(&patterns);
    CHECK(document == "10:a1\n20:b3\n30:c5");
    CHECK(beginCount == 1 && endCount == 1 && warningCount == 0);

    StringCchCopyW(patterns.pwsz1, COUNTOF(patterns.pwsz1), L"${1+*2}");
    patterns.pwsz2[0] = L'\0';
    document = "a\nb";
    selectionStart = selectionEnd = anchorPos = currentPos = 0;
    EditModifyLines(&patterns);
    CHECK(document.find("<???>") != std::string::npos);
    CHECK(beginCount == 2 && endCount == 2);

    multiSelection = true;
    EditModifyLines(&patterns);
    CHECK(beginCount == 2 && endCount == 2 && warningCount == 1);
    multiSelection = false;
    failReplace = true;
    CHECK(TryEditModifyLines(&patterns));
    CHECK(beginCount == 3 && endCount == 3);
    std::puts("TinyExpr line-numbering checks passed.");
    return 0;
}
