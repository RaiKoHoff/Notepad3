# C++ migration handoff

The authoritative remaining work and decisions are in [cpp-migration.md](cpp-migration.md); the detailed conversion and validation record is in [cpp-migration-done.md](cpp-migration-done.md). Read `AGENTS.md` and the relevant agent-trail guidance before continuing. This handoff is a snapshot, not permission to bypass the plan's decision gates.

## Next session

1. Evaluate the Lexilla factory as the next task, before DynStrg selection. `src\Styles.cpp` uses the C-linkage `CreateLexer`, which also provides the patched Lexilla catalogue. A C++-only route is possible in principle through `Lexilla::LexerModule::Create()`, but could duplicate the catalogue or require vendored changes. `LexillaAccess::MakeLexer` still calls a C-style factory. Characterize alternatives and their benefits before proposing a change; obtain approval before implementation or vendored edits.
2. Then map DynStrg/PathLib ownership, mutable buffers, aliasing and allocation failures and measure replacement candidates. The user approved eliminating null-versus-empty text distinction, not a particular replacement. `std::wstring` is only a comparison baseline. Preserve the static CRT, avoid new redistributable DLLs, and review new Windows imports individually.
3. Finish focused modernization and the plan's build, runtime and review gates. Keep `src\SciCall.h` during this plan; evaluating `ScintillaCall` is a post-plan open point.

## Completed state and limits

- Main first-party C translation units and lexer definitions have been converted to C++; bundled C libraries remain outside that conversion. Notepad3-specific TinyExpr callers use the C++ adapter; the old C facade was removed.
- Notepad3 and MiniPath use `/EHs`; explicit SEH guards remain, and C++ destructors are not guaranteed during SEH. Isolated Debug and Release Win32/x64 linked builds passed before the latest UCHARDET change. ARM64 and AVX2 completion builds, GUI behavior, and callback/allocation-failure boundaries remain unverified.
- The application uses a stack-allocated C++ UCHARDET detector; the vendored C API is retained for other consumers. After migration, real-model parity tests passed 198 cases on both x86 and x64, including a separate injected charset-copy failure; the existing encoding regression passed. Fresh isolated linked Release Win32/x64 builds passed with expected PE architectures, detector object, libraries, base resources and 26 application MUIs. Prober-allocation failures and full editor-load behavior remain unverified.
- Local migration tests live under `test\test_modules` and are indexed by `RunModuleTests.ps1`; they are not in native CI. The CI encoding regression stubs the low-level detector, while the UCHARDET parity runner uses real models and a stub encoding mapping.

## Worktree and safety

- The branch is `cpp_dev_master`. Preserve any existing tracked or untracked work; recheck the worktree at the start of the new session. The pre-existing `.agents\`, `_tmp\`, two untracked `Build\scripts` Python files, `requirements.txt`, and `test\GBK*.txt` were not part of this migration.
- A mistyped MSBuild isolation import caused a Win32 build to regenerate shared `Bin\Release_x86_v145` artifacts, including application and MiniPath binaries, the language base DLL and 26 application MUIs. They were not restored or cleaned. Future builds must verify the exact import file and per-project output and post-build destinations before execution; see `agent-trail\build-and-test\windows-build-and-test.md`.
- The existing `_tmp` resampler draft is untracked user work, is not a drop-in replacement, and is not included in the migration commit. Its static review and unverified runtime behavior are recorded in the completed-work plan.
- Do not assume approval for GUI tests, shared outputs, dependency installation or restoration, other build configurations, or changing vendored patches. Preserve the alternate `other_sln\Notepad3DLL` project's existing out-of-scope status.
