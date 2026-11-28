# Focused Native Tests

When to read: Selecting native regression runners or interpreting their coverage

Run commands from the repository root; discover Visual C++ build tools as described in [Windows build and test](windows-build-and-test.md). Stop on missing prerequisites. Do not clean existing user outputs or run GUI tests on a shared desktop without approval.

```powershell
pwsh -NoProfile -File test\test_files\encoding\TestEncodingDetection.ps1
$tests = 'test\test_modules'
pwsh -NoProfile -File "$tests\TestOpenDialogDirectory.ps1"
pwsh -NoProfile -File "$tests\TestCreateDirectory.ps1"
pwsh -NoProfile -File "$tests\TestLexerDialogSorting.ps1"
pwsh -NoProfile -File "$tests\TestZoomTooltipPosition.ps1"
pwsh -NoProfile -File test\TestWebTemplates.ps1
# Run all local module tests, or choose an individual runner and architecture.
pwsh -NoProfile -File "$tests\RunModuleTests.ps1"
pwsh -NoProfile -File "$tests\TestTinyExpr.ps1" -Architecture x64
pwsh -NoProfile -File "$tests\TestExceptionPolicy.ps1" -Architecture x64
pwsh -NoProfile -File Build\scripts\Test-Docs.ps1
```

The lexer-sorting (`src\Styles.cpp`), zoom call-tip (`src\Notepad3.cpp`) and WebTemplate (`src\Notepad3.cpp`, linking `src\DynStrg.cpp`) runners extract functions into temporary harnesses and remove them afterwards. The lexer-sorting and zoom call-tip runners create hidden Win32 windows; obtain approval before running either or the aggregate module runner on a shared desktop. Native CI runs neither these nor the other `test\test_modules` runners.

When renaming source files, update filename-based extraction runners and, for tests in CI, `.github\workflows\native-tests.yml` path filters so tests and CI triggers follow the new paths.

The encoding runner compiles current detector functions in a temporary native harness and removes its generated files. It requires Visual C++ build tools and stubs application state and UCHARDET; it does not test the full editor load path. See [Encoding diagnostics and tests](../domain/encoding-diagnostics-and-tests.md#regression-coverage).

The create-directory runner extracts `Path_CreateDirectoryEx` (`src\PathLib.cpp`), `CreateIniFile` (`src\Config\Config.cpp`), `_BuildSnapshotPath` and the directory-recreation block (`src\Notepad3.cpp`) into a temporary harness. It uses real temporary directories and files, stubs only the path/string handles and UI hooks, and injects raw `SHCreateDirectoryExW` statuses for failures that cannot be reproduced reliably. Its checks cover created, existing-directory, conflicting-file, and failure results. `-PathLibSource <file>` substitutes an alternate source, e.g. a pre-fix copy, to demonstrate a regression failing. Extraction is text-based, so renaming those functions requires updating the runner.

The Open-directory runner also requires Visual C++ build tools. It extracts `_GetParentDir`, `_CanonicalizeInitialDir`, and `_ResolveOpenInitialDir` from `src\Dialogs.cpp` into a temporary harness with in-memory path/MRU stubs. Its cases cover precedence, missing recent files/directories, drive roots, quoted relative history, and the Save/history distinction. It does not exercise real PathLib/filesystem behavior, environment expansion, `OpenFileDlg` aliasing, or the Windows dialog UI.

The resampler runner links the production implementation to a C caller and uses off-screen GDI bitmaps. It checks x86/x64 pixel snapshots and selected filter, dimension and error behavior; it does not exercise the dialog or allocation-failure paths.

The lexer-layout runner links the C/C++ lexer data to a C caller and exercises the extracted `Style_FillRelatedStyles` with stubbed style calls. It checks the sentinel, row lookup and related-style dispatch, not actual Scintilla rendering.

The EditLexer runner links the production unit to a C caller with stubbed Scintilla direct calls. It checks comment delimiters, folding availability and property dispatch; it does not exercise Scintilla rendering or editor UI.

The encoding-unit runner links the production unit to a C caller with a stub encoding table and unused application dependencies. It covers mapping, lookup, flags, BOMs, UTF-8 validation and a Base64 round trip, not the editor load path or encoding UI.

The language-unit runner links the production unit to a C caller and a test resource. It checks locale lookup, language-setting persistence decisions with INI stubs, fallback strings and formatting, not real language-pack loading or menus.

The dynamic-string runner links the production unit to a C caller with test-only heap fault injection. It covers initialization, failed growth, mutation, UTF-8 conversion and command-line escaping, not every allocation-failure path or PathLib integration.

The PathLib runner links production PathLib and DynStrg to C and C++ callers. It covers path comparison, normalization, quoting, relative and UNC paths, a long path and known-folder lookup; it does not exercise shell-link or folder-dialog UI.

The Helpers runner links production Helpers, PathLib and DynStrg to a C caller. It covers string/path parsing, escaping and a controlled worker cancellation; unrelated application callbacks are failure stubs, and AppID and GUI behavior are not exercised.

The Dlapi runner links production Dlapi, PathLib and DynStrg to a C caller. It checks filters and display names using shell items, plus PIDL size and concatenation; worker calls are failure stubs, and list-view UI behavior is not exercised.

The Styles runner extracts the production lexer registry, selection functions and style-backup copy callbacks, then links them with lexer definitions and a C caller. It covers shebang, name, extension and wildcard matching and backup copying; editor integration and UI are not exercised.

The FindReplace runner extracts search-target, wildcard and replacement-escape functions and links them with production Helpers and a C caller. It uses scripted Scintilla stubs; real search, PCRE2 and dialog behavior are not exercised.

The Dialogs runner extracts default-button mapping and file-filter conversion and links them to a C caller. It stubs allocation and does not exercise COM or dialog UI.

The Edit runner extracts EOL detection, numeric helpers, selection/undo sequencing and logical-sort comparators. It uses scripted Scintilla stubs and real Windows logical string comparison; it does not exercise actual Scintilla, file I/O, TinyExpr or GUI behavior.

The Notepad3 runner extracts expression-result formatting and links it to a C caller with the TinyExpr bit-width query stubbed. It covers decimal, hexadecimal, binary, logical and bounded-buffer formatting, not expression parsing, startup or editor UI.

The TinyExpr runner links the real interface and library to a C++ caller; see [TinyExpr++ and the Notepad3 interface](../domain/tinyexpr-wrapper.md#test) for coverage and locale limits.

The exception-policy runner checks evaluated project settings and exercises C++ unwinding through an `extern "C"` function plus an explicit SEH `__finally` guard. The ARM64 check is MSBuild evaluation only, not a build or runtime test.
