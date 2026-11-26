# Source Layout and External Tools

When to read: Main source layout, menu dispatch, or bundled external tools

## Project

Notepad3 is a Windows-only Win32 C/C++ text editor built on Scintilla/Lexilla, distributed under BSD-3. It includes MiniPath (`Ctrl+M`) and integrates the pre-built portable grepWin tool (`Ctrl+Shift+F`). The solution is `Notepad3.sln`; builds use Visual Studio 2026 and toolset v145.

## Source map

| File | Purpose |
|---|---|
| `src\Notepad3.cpp/h` | `wWinMain`, `MainWndProc`, global state, `MsgCommand()`, toolbar setup, word-wrap configuration, middle-click auto-scroll |
| `src\Edit.cpp/h` | Encoding, clipboard, indent, sort, bookmarks, folding, autocomplete, URL/color highlighting |
| `src\FindReplace.cpp/h` | Find/Replace dialog, search state, PCRE2-backed matching, replacement, occurrence marking |
| `src\Styles.cpp/h` | Scintilla styling, lexer selection, themes, margins |
| `src\Dialogs.cpp/h` | Dialogs and DPI-aware UI |
| `src\Config\Config.cpp/h` | INI load/save and MRU |
| `src\Encoding.cpp/h` | Character encoding detection/conversion |
| `src\SciCall.h` | Type-safe wrappers for Scintilla direct calls |
| `src\DynStrg.cpp/h` | `HSTRINGW` dynamic wide-string handle |
| `src\PathLib.cpp/h` | `HPATHL` path handle and long-path-aware Win32 wrappers |
| `src\TypeDefs.h` | `DocPos`, `DocLn`, `cpi_enc_t`, OS targeting, compiler macros |
| `src\MuiLanguage.cpp/h` | MUI language DLL loading |
| `src\StyleLexers\styleLex<name>.c` / `.cpp` | Per-language `EDITLEXER` definitions |

## Menu and command dispatch

- `MsgInitMenu()` (`WM_INITMENU`) controls menu enable/check state.
- `MsgCommand()` (`WM_COMMAND`) delegates to static sub-handlers that return `true` when they handle a command:

| Handler | Scope |
|---|---|
| `_HandleFileCommands` | `IDM_FILE_*` |
| `_HandleEncodingCommands` | `IDM_ENCODING_*`, `IDM_LINEENDINGS_*` |
| `_HandleEditBasicCommands` | `IDM_EDIT_UNDO`..`CMD_VK_INSERT` |
| `_HandleEditLineManipulation` | `IDM_EDIT_ENCLOSESELECTION`..`IDM_EDIT_INSERT_GUID` |
| `_HandleEditTextTransform` | `IDM_EDIT_LINECOMMENT`..`IDM_EDIT_HEX2CHAR` |
| `_HandleEditFind` | `IDM_EDIT_FINDMATCHINGBRACE`..`IDM_EDIT_GOTOLINE` |
| `_HandleViewAndSettingsCommands` | `IDM_VIEW_*`, `IDM_SET_*` |
| `_HandleHelpCommands` | `IDM_HELP_*` |
| `_HandleCmdCommands` | `CMD_*` |
| `_HandleToolbarCommands` | `IDT_*` through `s_ToolbarDispatch[]` |

## Libraries and external tools

- `scintilla\` and `lexilla\`, each with NP3 patch records in its `np3_patches` folder and offline documentation in its `doc` folder. Read their current [Scintilla](../../scintilla/version.txt) and [Lexilla](../../lexilla/version.txt) version files instead of relying on copied version numbers.
- `scintilla\pcre2\` ([version](../../scintilla/pcre2/version.txt)), `src\uchardet\`, `src\tinyexprcpp\`, `src\uthash\`, and `src\crypto\` (Rijndael/SHA-256 for AES-256).
- grepWin is external and is not built from source. Pre-built executables are in `grepWin\portables\`; language files are in `grepWin\translations\`.
  - Runtime lookup in `src\Dialogs.cpp`: `Settings2.GrepWinPath` → `<ModuleDir>\grepWin\grepWin-x{64,86}_portable.exe` → `%APPDATA%\Rizonesoft\Notepad3\grepWin\`.
  - `grepWinLangFileName[]` and `grepWinLangFileNamePortableApps[]` in `MuiLanguage.cpp` pair locales with `.lang` filenames. `DialogGrepWin` currently uses the matched locale field as the default `languagefile` value, then resolves the configured value before saving it ([source](../../src/Dialogs.cpp#L5519-L5559)).
  - `Build\make_portable_*.cmd` packages both portable executables, LICENSE, and all `.lang` files.
  - ARM64 uses the x64 executable via emulation (`#if defined(_M_ARM64)` in `Notepad3.cpp`).
