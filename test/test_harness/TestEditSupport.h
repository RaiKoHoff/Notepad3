#pragma once
#include "TypeDefs.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <shlwapi.h>
#include <wchar.h>

#define COUNTOF(ar) ARRAYSIZE(ar)
static __forceinline DocLn max_ln(DocLn x, DocLn y) { return x < y ? y : x; }
static __forceinline bool StrIsEmptyW(LPCWSTR s) { return !s || *s == L'\0'; }
#define StrIsNotEmpty(s) (!StrIsEmptyW(s))
static __forceinline int wcscoll_s(const wchar_t* a, const wchar_t* b) {
    return (a && b) ? wcscoll(a, b) : (a ? 1 : (b ? -1 : 0));
}

#ifdef __cplusplus
extern "C" {
#endif
LONG BeginUndoActionSelection(void);
void EndUndoActionSelection(LONG token);
bool TestSelectionEmpty(void);
DocPos TestSelectionStart(void);
void TestReplaceSel(const char* text);
DocPos TestCurrentPos(void);
void TestSetSel(DocPos start, DocPos end);
void EditReplaceSelection(const char* text, bool force);
void EditDetectEOLMode(LPCSTR data, size_t length, EditFileIOStatus* status);
bool TestModifyDec(const char* text, bool increase, char* out, size_t size);
bool TestModifyHex(const char* text, bool increase, char* out, size_t size);
bool TestModifyBin(const char* text, bool increase, char* out, size_t size);
int TestLogicalCompare(wchar_t* line1, wchar_t* key1, wchar_t* line2, wchar_t* key2, bool reverse);
#ifdef __cplusplus
}
#endif

#define UndoTransActionBegin() { LONG _token_ = 0L; __try { _token_ = BeginUndoActionSelection();
#define EndUndoTransAction() ;} __finally { EndUndoActionSelection(_token_); } }
#define SciCall_IsSelectionEmpty TestSelectionEmpty
#define SciCall_GetSelectionStart TestSelectionStart
#define SciCall_ReplaceSel TestReplaceSel
#define SciCall_GetCurrentPos TestCurrentPos
#define SciCall_SetSel TestSetSel
