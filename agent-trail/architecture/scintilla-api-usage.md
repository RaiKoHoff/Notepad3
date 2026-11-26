# Scintilla API Usage Notes

When to read: Scintilla message replacements, zoom call-tip positioning, or Scintilla upgrades

Constraints for first-party Scintilla calls and native zoom call-tip positioning. Wrapper rules are in [Codebase and conventions](codebase-and-conventions.md).

## Direct-call alternatives

- `ScintillaCall` is not a drop-in replacement for `SciCall`: its [implementation](../../scintilla/call/ScintillaCall.cxx#L35-L43) throws `Scintilla::Failure` for error statuses, while the application's [search handling](../../src/FindReplace.cpp#L608-L619) explicitly clears and checks sticky regex warnings. Warnings are not thrown by `ScintillaCall`. Its implementation is not a compile item in the current [Scintilla](../../scintilla/win32/Scintilla.vcxproj) or [Notepad3](../../src/Notepad3.vcxproj) project.
- On x86, do not cast the locally exported `Scintilla_DirectStatusFunction` to `Scintilla::FunctionDirect`: the [export](../../scintilla/win32/ScintillaWin.cxx#L4188-L4193) uses `SCI_METHOD` (`__stdcall`), whereas the [class pointer type](../../scintilla/include/ScintillaCall.h#L25) uses the default calling convention. If evaluating the class, obtain the matching function through [`SCI_GETDIRECTSTATUSFUNCTION`](../../scintilla/win32/ScintillaWin.cxx#L2272-L2279) instead, and respect its [window-thread requirement](../../scintilla/win32/ScintillaWin.cxx#L4158-L4165).

Evidence: source inspection of the working tree based on `d9cc3ac61c224ed7aebd199819f169fa5acd90a`; runtime parity was not tested.

## Reading document text

- Prefer one `SciCall_GetRangePointer(start, len)` per line or range over a `SciCall_GetCharAt` loop (one message per byte). The pointer is valid only until the next document modification; reacquire it after any edit.
- Byte-wise scanning for ASCII characters is safe in UTF-8 (`SciCall_GetCodePage() == SC_CP_UTF8`), because multi-byte sequences consist of bytes >= 0x80. For other code pages (DBCS trail bytes can equal `[`, `]`, `{`, `}`), step with `SciCall_PositionBefore/After`.

## Pitfalls of specific messages

| Message | Behaviour to respect |
|---|---|
| `SCI_POSITIONRELATIVE` | Clamped to `[0, Length]` in `scintilla\src\Editor.cxx`. An offset past the document end returns **0**, not the end. Compare the result with the start position before using it as a limit (see `EditGetExcerpt`). |
| `SCI_SETLINEINDENTATION` | Does nothing when the requested indentation width equals the current one (`Document::SetLineIndentation`, `scintilla\src\Document.cxx`). It cannot convert tabs to spaces (or back) at the same width. |
| `SCI_REPLACETARGETMINIMAL` | `Sci_ReplaceTargetTestChgHist` (`src\SciCall.h`) uses it only when change history is enabled; otherwise a full `SCI_REPLACETARGET`. |
| `SCI_MARKERNEXT` / `SCI_MARKERPREVIOUS` | Do not see change-history markers (21-24); `_MarkerNext` / `_MarkerPrevious` in `src\Edit.cpp` scan linearly for them. |

## Native zoom call-tip discovery

- Preserve the combined discovery checks in `_FindZoomCallTip` / `_PositionZoomCallTip`: enumerate the current thread's windows, match the editor's top-level owner (`GA_ROOT`), require class `CallTip`, and compare window extra bytes at offset zero with the nonzero `SciCall_GetDirectPointer()`. Windows promotes the child editor's popup owner to its top-level window; owner matching alone cannot distinguish editors sharing that window (both in `src\Notepad3.cpp`).
- Never cache the popup HWND across cancellation. `CallTip::CallTipCancel` destroys the native window; rediscover it after showing the next tip ([lifetime](../../scintilla/src/CallTip.cxx#L312-L317)).
- Revalidate this contract when upgrading Scintilla: the class and instance-pointer storage are vendored Win32 internals, not a public popup-HWND API. `ScintillaWin::CTWndProc` stores the instance via `SetWindowPointer`, whose offset-zero storage must still agree with `SCI_GETDIRECTPOINTER` ([creation](../../scintilla/win32/ScintillaWin.cxx#L4134-L4147), [storage](../../scintilla/win32/PlatWin.cxx#L78-L84), [direct pointer](../../scintilla/win32/ScintillaWin.cxx#L2278-L2279)).

Ownership isolation and popup recreation are exercised by `test\TestZoomTooltipPosition.cpp` using hidden Win32 windows. Those checks do not establish visible rendering or physical monitor-DPI behavior.

## Wrapped but unused

`SciCall_IsRangeWord` exists in `src\SciCall.h` but has no caller. See [open questions](../open-questions.md) before using it to replace delimiter scans.
