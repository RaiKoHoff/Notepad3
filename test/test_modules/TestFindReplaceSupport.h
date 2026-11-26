#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Helpers.h"
#include "Encoding.h"

DocPos SciCall_GetTargetStart(void);
DocPos SciCall_GetTargetEnd(void);
void SciCall_SetTargetRange(DocPos start, DocPos end);
void SciCall_SetSearchFlags(int flags);
DocPos SciCall_SearchInTarget(DocPos length, const char* text);
void SciCall_SetStatus(int status);
int SciCall_GetStatus(void);
DocPos SciCall_PositionAfter(DocPos pos);
DocPos SciCall_PositionBefore(DocPos pos);
bool Sci_IsPosValid(DocPos pos);
int SciCall_GetChangeHistory(void);

DocPos TestFindInTarget(LPCWSTR text, int flags, DocPos* begin, DocPos* end, bool force, int mode);
void TestTransformReplacement(LPSTR text, bool regex, int* message);
void TestEscapeWildcards(LPWSTR text, size_t capacity, LPEDITFINDREPLACE data);

#ifdef __cplusplus
}
#endif
