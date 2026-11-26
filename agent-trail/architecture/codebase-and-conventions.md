# Codebase and Conventions

When to read: Menus, commands, source layout, coding conventions, buffers

For the source map, command dispatch, and bundled tools, see [Source layout and external tools](source-layout.md).

## Coding conventions

- Follow `.clang-format` in `src\`: four-space indent, Stroustrup braces, left-aligned pointers, no column limit, no include sorting. `.editorconfig` requires UTF-8/CRLF; preserve tabs in upstream Lexilla code.
- Use `strsafe.h`; deprecated string functions are disabled.
- Use `DocPos` / `DocPosU` / `DocLn` for document positions/lines, `cpi_enc_t` for encodings, and `HSTRINGW` / `HPATHL` instead of raw wide-string buffers. `NOMINMAX` is global; use typed equivalents or `min()` / `max()`.
- Use existing global state structs (`Globals`, `Settings`, `Settings2`, `Flags`, `Paths`); do not add globals.
- C-style COM `lpVtbl` calls require the corresponding interface form in C++; preserve ownership and calling conventions ([directory list](../../src/Dlapi.cpp#L215-L248), [file dialog](../../src/Dialogs.cpp#L7570-L7598)).
- Always use `SciCall.h` wrappers for Scintilla calls; add a missing wrapper there rather than using `SendMessage`. Naming: `DeclareSciCall{V|R}{0|01|1|2}` (`V`=void, `R`=return; `01`=lParam-only). The message argument is the suffix after `SCI_`.

```c
DeclareSciCallV0(Undo, UNDO);                                            // SciCall_Undo()
DeclareSciCallV1(SetTechnology, SETTECHNOLOGY, int, technology);         // SciCall_SetTechnology(int)
DeclareSciCallR0(GetTextLength, GETTEXTLENGTH, DocPos);                  // DocPos SciCall_GetTextLength()
DeclareSciCallR1(SupportsFeature, SUPPORTSFEATURE, bool, int, feature);  // bool SciCall_SupportsFeature(int)
```

- Group undoable edits with `UndoTransActionBegin()` / `EndUndoTransAction()` (`Notepad3.h`); they suppress document-change notifications only at the outermost undo level. For bulk edits that must also suppress redraw, nest `LimitNotifyEvents()` / `RestoreNotifyEvents()`.
- These guards use MSVC SEH `__try` / `__finally`. Keep C++ objects requiring destruction in a separate helper: MSVC rejects a function combining them with `__try` (C2712). See [`EditModifyLines` / `_EditModifyLinesImpl`](../../src/Edit.cpp#L3562-L3849) and the [guard macros](../../src/Notepad3.h#L185-L186).
- `InfoBoxLng()` returns `MAKELONG(button, mode)`. When a suppression key is non-NULL, a saved-answer replay sets the high word; do not compare the raw result with `IDNO`. Use `IsYesOkay()`, `IsRetryContinue()`, `IsNoCancelClose()`, or extract `INFOBOX_ANSW(r)` / `INFOBOX_MODE(r)`.
- For new Scintilla/Lexilla APIs, consult the vendored versions in `scintilla\version.txt` and `lexilla\version.txt` and the offline docs under `scintilla\doc\` and `lexilla\doc\`.

### Local patch guidance

Before an approved vendored-library change, consult the [Lexilla patch registry](../../lexilla/np3_patches/README.md) and relevant Scintilla records: [DirectWrite font resolution](../../scintilla/np3_patches/001_directwrite_font_resolution.md), [font weight/style enums](../../scintilla/np3_patches/002_fontweight_fontstyle_enums.md), and [mouse-pointer scope](../../scintilla/np3_patches/003_mousevanish_cursor_scope.md). Verify each against current sources; these records are preservation guidance, not a complete or freshly certified patch inventory.

`scintilla\np3_patches\upstream_558\` is an older import record. Historical plans under `plans\` are not active instructions or permission for product refactoring.

### Find/Replace dialog text limits

- `CB_LIMITTEXT` (`FNDRPL_BUFFER`) does not apply to `WM_SETTEXT`; bound text before calling `ComboBox_SetText*`, or an oversized value freezes the single-line combo edit (see the `bFindReplCopySelOrClip` block in `EditFindReplaceDlgProc`, `src\FindReplace.cpp`).
- `ComboBox_SetText*` fires `CBN_EDITCHANGE` synchronously; reset one-shot flags before calling it.
- `_FindInTarget` has an 8192-byte `chFind` buffer; a longer pattern makes `WideCharToMultiByte` fail (`len` = -1) and Scintilla reports "not found".

### Find/Replace dialog placement

- For live recreation, capture the old screen rectangle and restore against its monitor. [`SetDlgPos`](../../src/Dialogs.cpp#L6166) is for owner-relative settings and clamps to the owner's monitor; [`_RestoreFindReplaceDlgPosition`](../../src/FindReplace.cpp#L830) uses the old rectangle's monitor and fits the dialog again after any DPI resize.

### Dynamic string/path buffer lifetime

`StrgCreate(NULL)` can return a valid handle without allocating its text buffer; `StrgGet(h) == NULL` before initialization is normal, not proof of allocation failure. Initialize the fresh handle with `StrgReset(h, text)` and check its result (`> 0`) before using the text ([source](../../src/DynStrg.cpp#L432-L473): `StrgCreate`, `StrgReset`, `StrgGet`).

Never use a pointer returned by `Path_WriteAccessBuf()` / `StrgWriteAccessBuf()` after an operation that may reallocate or swap the same handle's buffer. Examples include `Path_CanonicalizeEx`, `Path_Swap` / `StrgSwap`, `Path_ExpandEnvStrings` / `ExpandEnvironmentStrgs`, `Path_Append` / `Path_Reset`, `StrgCat` / `StrgInsert` / `StrgFormat` / `StrgReset`, and `Path_NormalizeEx` / `Path_AbsoluteFromApp` / `Path_RelativeToApp`.

- For read-only access afterward, use `Path_Get(h)` / `StrgGet(h)`.
- For read-write access, reacquire with `Path_WriteAccessBuf(h, 0)` / `StrgWriteAccessBuf(h, 0)`; size 0 does not resize.
