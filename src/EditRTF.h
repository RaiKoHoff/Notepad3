// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* EditRTF.h                                                                   *
*   Copy current selection (or whole document) to clipboard as RTF,           *
*   preserving Scintilla syntax highlighting.                                 *
*   Based on Notepad4's SaveToStreamRTF (in turn based on SciTE's             *
*   ExportRTF.cxx).                                                           *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_EDIT_RTF_H_
#define _NP3_EDIT_RTF_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// Copy current selection as RTF + UTF-16 plain text to clipboard.
// If selection is empty, falls back to the whole document.
// No-op on empty document.
void EditCopyAsRTF(HWND hwnd);

#ifdef __cplusplus
}
#endif

#endif // _NP3_EDIT_RTF_H_
