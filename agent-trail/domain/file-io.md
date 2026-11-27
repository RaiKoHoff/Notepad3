# File I/O

When to read: `Edit.cpp` load/save, atomic save, file watching, Open/MRU dialogs

## Load and save path

`FileSave()` / `FileLoad()` in `Notepad3.cpp` call `FileIO()`, which dispatches to `EditSaveFile()` / `EditLoadFile()` in `Edit.cpp`.

## Scintilla read-only state

Route persistent main-editor read-only changes through `UpdateDocumentReadOnlyState()` in `Notepad3.cpp`, including buffer preparation in `_PrepareDocBuffer()` (`Edit.cpp`). Directly setting Scintilla's read-only flag there can drop another active cause, such as file-attribute or saved View-only mode. Temporary programmatic unlocks must restore the prior read-only state.

## Saving

- Atomic save uses a temporary file and `ReplaceFileW`, controlled by `Settings2.AtomicFileSave`.
- Atomic save is skipped for emergency session-end saves (`FSF_EndSession`).
- If temp-file setup fails, saving falls back to writing the target directly. After a successful temp-file write, replacement tries `ReplaceFileW` for an existing target, then `MoveFileExW` with replace/write-through flags; a replacement failure makes the save fail.
- `Globals.dwLastError` holds the Win32 error. In `FileSave()`, check `ERROR_ACCESS_DENIED` and `ERROR_PATH_NOT_FOUND` before the generic fallback.

## Open-dialog initial directory

- `OpenFileDlg()` / `_ResolveOpenInitialDir()` in `src\Dialogs.cpp` prefer an explicit initial path, then the current file's directory, `Settings.DefaultDirectoryOverride`, and `Settings2.DefaultDirectory`.
- With none of those available and `Settings.SaveRecentFiles` enabled, Open uses the newest MRU entry's parent directory if it exists (the file itself need not exist). Otherwise it uses `Paths.WorkingDirectory`. It does not search older MRU entries.
- MRU paths must be sanitized, unquoted, and expanded with `Path_AbsoluteFromApp(..., true)` before taking the parent directory with the drive-root-safe `_GetParentDir()`. An unavailable parent clears the candidate so the shared resolver supplies the working directory. Save dialogs do not use history.
- An invalid final directory still falls back to Documents; Save also requires a writable directory. Focused resolver coverage: `pwsh -NoProfile -File test\test_modules\TestOpenDialogDirectory.ps1`; see [focused native tests](../build-and-test/focused-native-tests.md) for its in-memory stub boundaries.

## File watching

`InstallFileWatching()` uses `FindFirstChangeNotificationW` on the parent directory. Stop watching with `InstallFileWatching(false)` before saving, then reinstall it with `InstallFileWatching(true)` afterward.

## Recent files dialog (`FileMRUDlgProc`, `Dialogs.cpp`)

- OK on a missing entry only offers removal (`IDS_MUI_ERR_MRUDLG`). Recovery actions live in the list's context menu (right-click / Shift+F10, `_FileMRU_ContextMenu()`): open containing folder, browse for renamed/moved file, copy full path / directory / file name (`IDC_MRU_*`, `IDS_MUI_MRU_*`).
- Folder actions walk up to the nearest existing parent with `_FindExistingParentDir()` in `src\Dialogs.cpp`. "Browse" opens `OpenFileDlg()` there, deletes the old MRU entry (unless it is the current file) and returns the chosen path through the dialog's `DWLP_USER` `HPATHL` (`EndDialog(IDOK)`), so `IDM_FILE_RECENT` in `Notepad3.cpp` loads it.
- Keep the MRU dialog's scratch `hFilePath` on the UI thread. `FileMRUDlgProc` gives each icon job owned path copies before starting it; `FileMRULookupIcon()` does not borrow dialog storage.
- Shell APIs need the `\\?\` prefix stripped: use `ShellRevealInExplorer()` / `_PathForShell()` in `Dialogs.cpp`.
