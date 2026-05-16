# Plan — Follow-up enhancements: file/path reference handling

Companion to the now-shipped change that added `IDM_EDIT_OPEN_CONTAINING_FOLDER_SEL` / `IDM_EDIT_OPEN_FILE_FROM_SEL` and fixed the `Paths.ModuleDirectory` anchor bug in the hyperlink-click handler.

These items were intentionally deferred from that change to keep the scope tight. They are independent of each other; pick any subset.

## Status (2026-05-16)

Already shipped on `dev_master`:
- New selection-based commands `IDM_EDIT_OPEN_CONTAINING_FOLDER_SEL` (40395) and `IDM_EDIT_OPEN_FILE_FROM_SEL` (40396), wired to Edit > Miscellaneous and the editor right-click popup. No accelerator — users can bind one via custom keybindings if desired.
- Shared helpers in `src\Helpers.c`: `ExtractSelectionOrTokenAtCaret`, `SplitFilePathLineColNum`, `ResolveSelectionToPath`.
- Bug fix at `src\Notepad3.c:8891` (or thereabouts after later edits) — relative `file:///` URLs now anchor to the current document's directory, fallback `Paths.WorkingDirectory`, never `Paths.ModuleDirectory`.
- English placeholder strings replicated across all 25 sibling locale `menu_*.rc` files. Translators still need to translate them.

Function bodies for the two new commands live in `src\Notepad3.c` (not Edit.c) because `<shlobj.h>` defines `SORT_ASCENDING` / `SORT_DESCENDING` enum values that conflict with Edit.c's `SortOrderMask` enum.

## Item 1 — Bare-path hotspotting (`HYPLNK_REGEX_FULL` extension)

**Goal**: make `C:\Users\me\foo.txt`, `\\server\share\file`, `.\rel\path.c`, `../include/foo.h`, and similar bare paths clickable hotspots in the editor, just like `https://...` URLs already are.

### Why this is deferred and load-bearing

The existing hyperlink hotspot system (`src\Edit.c:111-116`) uses one big PCRE2 regex that requires an explicit scheme prefix:
```c
#define HYPLNK_REGEX_FULL   "\\b(?:(?:https?|ftp)://|file:///|file://|mailto:|www\\.|ftp\\.)" ...
```
Adding bare-path matching is high-risk because:
1. **False positives** — `C:` could match in non-path text (e.g. a code snippet, a colon-time `12:34:56`).
2. **Indicator-marking pass cost** — the URL hotspotter runs on every restyle (`MarkAllOccurrences` flow). A broader regex adds CPU per keystroke in large documents.
3. **Lexer interference** — many lexers already style strings/paths in domain-specific ways. Adding `INDIC_NP3_HYPERLINK` on top of an existing lexer style can cause visual conflicts (double-underlining, color clash).
4. **`SCN_INDICATORRELEASE` activation** — currently `HandleHotSpotURLClicked` at `src\Notepad3.c:8824` is dispatched on indicator-release. It calls `UrlIsFileUrl(szTextW)` to decide between OPEN_IN_NOTEPAD3 and OPEN_WITH_BROWSER. Bare paths aren't file:// URLs, so the routing logic needs a new branch: "if it looks like a Windows path, treat it as OPEN_IN_NOTEPAD3 directly."

### Implementation sketch

Two viable approaches:

**A. Single bigger regex.** Add an alternation arm for paths. E.g.:
```
\\b(?:[A-Za-z]:[\\\\/]|\\\\\\\\[^\\s]+\\\\|\\.{1,2}[\\\\/])[^\\s'`"<>|*,;]+
```
- Drive-letter: `C:\...`, `c:/...`
- UNC: `\\server\share\...`
- Relative: `.\foo`, `..\foo`, `./foo`, `../foo`

Pros: minimal code change. Cons: regex becomes harder to maintain; false-positive risk is concentrated.

**B. Second-pass detection.** Keep `HYPLNK_REGEX_FULL` for scheme URLs. Add a separate, more restrictive pass for bare paths only when the line context suggests "this is a path" — e.g. preceded by `at`, `file:`, `> `, in compiler-error format `file.c:42:`, etc. More conservative; harder to author.

**Recommendation**: try (A) but gate behind a `Settings2.HyperlinkBarePaths` opt-in toggle defaulting to `false`. Document the toggle in `Build\Notepad3.ini` `[Settings2]` and `readme/config/Configuration.md` (CLAUDE.md invariant).

### Required touchpoints

| File | Change |
|---|---|
| `src\Edit.c:111-116` | Extend `HYPLNK_REGEX_FULL` (or add `HYPLNK_REGEX_BAREPATH` and run both) |
| `src\Notepad3.c:8824` (`HandleHotSpotURLClicked`) | New branch when `!UrlIsFileUrl(szTextW)` AND text looks like a Windows path → call same OPEN_IN_NOTEPAD3 logic but skip `PathCreateFromUrlW` (use the text as-is, then `ResolveSelectionToPath`-style anchoring) |
| `src\TypeDefs.h` (`Settings2`) | Add `HyperlinkBarePaths` bool with default `false` |
| `src\Config\Config.cpp` | Persist `HyperlinkBarePaths` |
| `Build\Notepad3.ini`, `readme\config\Configuration.md` | Document new INI key |

### Verification

- Open a fresh `.txt` buffer with toggle ON. Paste:
  ```
  C:\Windows\notepad.exe
  ..\src\Helpers.c
  \\?\C:\Windows
  src/Edit.c:111
  See line C: this should NOT highlight
  Time 12:34:56 must NOT highlight
  ```
  Expect: lines 1-4 hotspotted, lines 5-6 not.
- With toggle OFF: no change vs. current behavior.
- Test in C/C++ buffer (SCLEX_CPP) to confirm no lexer-style clash on strings containing paths.

## Item 2 — Markdown `[label](url)` extraction in `ExtractSelectionOrTokenAtCaret`

**Goal**: when the caret is inside the URL portion of a Markdown link `[label](url)`, the selection extractor should grab `url` rather than expanding outward through the closing `)` and stopping at the next whitespace.

### Why this is deferred

The current `ExtractSelectionOrTokenAtCaret` in `src\Helpers.c` uses a flat boundary set `[\s'`"<>|*,;]` and never considers paired-delimiter context. Notepad4's `EditGetStringAroundCaret` (`notepad4\src\Edit.cpp:7015-7093`) has a dedicated subroutine to recognize `(...)` after a Markdown link target and clamp the extraction.

It's deferred because: (a) Notepad3 already hotspots Markdown URLs via the lexer-driven indicator path, so most Markdown click flows already work; (b) the extractor is used by the new selection commands, where the user can always select the URL by hand if the auto-expand is wrong.

### Sketch

Inside `ExtractSelectionOrTokenAtCaret` in `src\Helpers.c`, when the selection is empty, BEFORE the regular boundary scan:

```c
// Markdown link target: if caret sits inside `(...)` and the char before '(' is ']',
// clamp the extraction to the contents of the parens.
DocPos const posCur = SciCall_GetCurrentPos();
// Walk back to find an unmatched '(' on the same line.
// If found AND the char immediately before is ']', use [openParen+1, matching ')') as the span.
```

Use Scintilla's bracket-matching (`SciCall_BraceMatch`) — but only on `(`; Scintilla supports `()` `[]` `{}` `<>`. Caveat: caret-on-`(` semantics vary; you may need to do a manual scan.

### Touchpoints

- `src\Helpers.c` — `ExtractSelectionOrTokenAtCaret` (introduced in the prior change).

### Verification

```markdown
See [the doc](D:\projects\foo\readme.md) for details.
                ^ caret here
```
Invoke "Open File from Selection" → opens `D:\projects\foo\readme.md`. Currently expands to `D:\projects\foo\readme.md)` and the trailing `)` may or may not be tolerated by `ResolveSelectionToPath`.

## Item 3 — Enable/disable gating in `MsgInitMenu` for the new commands

**Goal**: grey out `IDM_EDIT_OPEN_CONTAINING_FOLDER_SEL` / `IDM_EDIT_OPEN_FILE_FROM_SEL` when there's no selection AND no token at the caret position.

### Current behavior

The commands are always enabled. Invoking them with an empty selection on whitespace-only content silently produces a `MessageBeep(MB_ICONHAND)`. This matches Notepad4's UX and is acceptable, but a power user might prefer a greyed-out menu.

### Sketch

`MsgInitMenu()` in `src\Notepad3.c` should add (near the existing `IDM_EDIT_INSERT_GUID` enable check around line 4963):
```c
bool const bHasToken = !SciCall_IsSelectionEmpty();
// optionally also check word boundaries at SciCall_GetCurrentPos()
EnableCmd(hmenu, IDM_EDIT_OPEN_CONTAINING_FOLDER_SEL, bHasToken /* OR token-at-caret */);
EnableCmd(hmenu, IDM_EDIT_OPEN_FILE_FROM_SEL, bHasToken);
```

Right-click context menu builder at `src\Notepad3.c:4549` (`imenu == MNU_EDIT` branch) already has a similar pattern for the case-conversion commands using `s_iCtxMenuClickPos`. Add the same predicate there.

### Verification

- Empty selection, caret on whitespace → menu items greyed.
- Empty selection, caret in a word → enabled.
- Any non-empty selection → enabled.

## Item 4 — `\\?\` long-path prefix handling in `ResolveSelectionToPath`

**Goal**: gracefully handle paths with the Windows long-path prefix when passed to `SHParseDisplayName`.

### Why

`SHParseDisplayName` does not always accept `\\?\C:\...`. For shell-API consumption the prefix should be stripped. PathLib wrappers already handle this internally for filesystem ops, but `EditOpenContainingFolderFromSelection` calls `SHParseDisplayName` directly with `Path_Get(hpth)` which may include the prefix.

### Sketch

Inside `EditOpenContainingFolderFromSelection` in `src\Notepad3.c`, before `SHParseDisplayName`:
```c
LPCWSTR pth = Path_Get(hpth);
if (StrCmpNW(pth, L"\\\\?\\", 4) == 0) {
    // Skip the prefix for shell-API use. If \\?\UNC\..., strip differently.
    if (StrCmpNW(pth + 4, L"UNC\\", 4) == 0) {
        // \\?\UNC\server\share → \\server\share
        WCHAR shellPath[MAX_PATH_EXPLICIT];
        StringCchPrintf(shellPath, COUNTOF(shellPath), L"\\\\%s", pth + 8);
        SHParseDisplayName(shellPath, ...);
    } else {
        SHParseDisplayName(pth + 4, ...);
    }
}
```

### Verification

- `\\?\C:\Windows` → opens `C:\Windows` in Explorer.
- `\\?\UNC\server\share\file.txt` → opens `\\server\share\` with `file.txt` selected.

## Item 5 — `#fragment` parsing for `file://` URLs

**Goal**: a `file:///D:/docs/readme.md#section-2` URL clicked in Notepad3 should at minimum open the file (currently works) and ideally jump to the line with `#section-2` as a heading anchor.

### Why deferred

Markdown anchor resolution is out of scope for a text editor — the heading-anchor mapping is a Markdown-specific convention. The minimum useful behavior is to strip `#...` before resolving the path. Currently `PathCreateFromUrlW` may or may not strip it depending on Windows version.

### Sketch

In `HandleHotSpotURLClicked` at `src\Notepad3.c:8824`, after `PathCreateFromUrlW`:
```c
LPWSTR const hash = StrChr(szUnEscW, L'#');
if (hash) {
    *hash = L'\0';  // discard fragment
}
```
A future Markdown-aware extension could parse `#anchor-name`, scan the buffer for `## Anchor Name` (case-insensitive, dashes→spaces), and `SCI_GOTOLINE` there. Probably not worth the complexity.

### Verification

Click `file:///D:/test/foo.md#whatever` — opens `foo.md`. Don't error or attempt to interpret `#whatever`.

## Item 6 — Localize the new menu strings (translator task, not Claude's)

The 25 sibling locales currently have **English placeholder** strings for the two new menu items. Translators need to update:

- `language\np3_*\menu_*.rc` — replace `"Open &Containing Folder of Selection\tCtrl+Shift+,"` and `"Open &File from Selection"` (both in Edit > Miscellaneous AND in `IDR_MUI_POPUPMENU`).

This is normal translator workflow — not blocked, just pending.

### What Claude can do

Nothing without authoritative translations. If asked to "translate", refuse and ask the user to source translations from the locale maintainers listed in `language\np3_*\strings_*.rc` (`IDS_MUI_TRANSL_AUTHOR`).

## Item 7 — Tinyexpr behavior in `SplitFilePathLineColNum` `:N` fallback

`SplitFilePathLineColNum` falls back to `SplitFilePathLineNum` (which uses `te_interp`) for the single-colon case `foo.c:42`. This means `foo.c:42+1` would parse as line 43 — a side effect of using tinyexpr. Two existing callers depend on this:

- `src\Notepad3.c:8666` — `SplitFilePathLineNum(wchUrl, NULL)` — only cuts the suffix, discards the number.
- `src\Notepad3.c:8879` — `SplitFilePathLineNum(szTextW, &lineNum)` — uses the parsed number.

If we ever want to drop the tinyexpr quirk: replace the wrapper with a plain `_wtoi` parse. Verify the two existing callers don't pass arithmetic.

## Touch-stones for any future session

- **CLAUDE.md** in repo root has all the build/encoding/localization invariants. Read it before touching `.rc` files.
- **`.venv\Scripts\python.exe`** is required for bulk locale edits. Do NOT use sed/perl.
- **`.rc` files** are UTF-8 *without* BOM, CRLF. Use `Build\rc_to_utf8.cmd` if a BOM sneaks in.
- **Locale es_la** has an empty directory — script-based bulk edits should skip it gracefully.
- **Reference impl**: Notepad4 at `D:\DEV\GitHub\notepad4\` — inspect `src\Edit.cpp` (`EditOpenSelection`, `EditGetStringAroundCaret`) and `src\Helpers.cpp` (`OpenContainingFolder`).
- **Build**: `Build\Build_x64.cmd Release` for quick verification. The Build.ps1 wrapper has quoting quirks under some PowerShell versions — invoke MSBuild directly if it fails: `& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "D:\DEV\GitHub\Notepad3\Notepad3.sln" /m /p:Configuration=Release /p:Platform=x64 /v:minimal`.

## Out of scope for these follow-ups

- IDS_MUI_* / Notepad3DLL string resources — none of the deferred items add user-facing strings beyond menu labels (which are inline in `.rc`).
- ARM64-specific concerns — none of these touch rendering or CET paths.
- grepWin integration — orthogonal.
