# Windows Build and Test

When to read: Build, MSBuild, CI, test scripts, Python scripting

Run commands from the repository root. Discover installed tools before assuming they are on PATH.

## Focused feedback

See [Focused native tests](focused-native-tests.md) for runner commands, safety boundaries, and coverage.

For compiler-only feedback on the main project, locate MSBuild:

```powershell
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -requires Microsoft.Component.MSBuild `
    -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
if (!$msbuild) { throw 'MSBuild not found' }
& $msbuild src\Notepad3.vcxproj /t:ClCompile /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $LASTEXITCODE" }
```

`ClCompile` is not a linked application build: it does not validate resource DLLs or integration with rebuilt dependencies. To isolate compiler outputs, add `/p:BuildProjectReferences=false` and override `IntDir` / `OutDir` with separate directories outside the checkout. Do not clean existing user build outputs.

## Linked builds and prerequisites

- When isolating a solution build with `ForceImportBeforeCppTargets`, verify that the exact target file exists and evaluate each project's output, intermediate and post-build destinations before starting. A mistyped, nonexistent import path was silently ignored in a Release/Win32 build, which wrote to the default shared `Bin` outputs instead ([application defaults](../../src/Notepad3.vcxproj#L180-L182)). Observed in the working tree based on `d9cc3ac61c224ed7aebd199819f169fa5acd90a`.
- [Notepad3](../../src/Notepad3.vcxproj#L261) and [MiniPath](../../minipath/minipath.vcxproj#L207) explicitly use `/EHs` in their project configurations (`SyncCThrow`); this supports standard C++ unwinding without assuming `extern "C"` functions cannot throw. In isolated Debug/Release Win32/x64 builds, Scintilla and Lexilla used `/EHsc`. The [exception-policy runner](../../test/test_modules/TestExceptionPolicy.ps1) checks the application settings and exercises C++ and explicit SEH behavior; it does not execute ARM64.
- Language-project post-build steps delete and regenerate MUI outputs relative to `TargetDir`; verify those destinations before redirecting or running a build against shared outputs ([example](../../language/np3_en_us/np3_en_us.vcxproj#L418-L421)).
- `Build\Build_x64.cmd [Release|Debug]` invokes the full solution for one platform; variants are `_Win32`, `_ARM64`, and `_x64_AVX2`. Default is Release; x64_AVX2 maps to platform x64 and configuration `Release_AVX2` / `Debug_AVX2` in `Build\scripts\Build.ps1`.
- For missing packages or a first build, use a current NuGet CLI: `nuget restore Notepad3.sln`. CI provisions it with `NuGet/setup-nuget`. The bundled `.nuget\NuGet.exe` is legacy (2.7); its restore compatibility has not been verified.
- `pwsh -NoProfile -File Version.ps1` regenerates `src\VersionEx.h` and `res\Notepad3.exe.conf.manifest` and updates version bookkeeping in `Versions\`. It changes local build state; do not run it solely for a documentation check.
- `Build\BuildAll.cmd` builds all four platforms; `Build\Clean.cmd` removes build outputs. Neither is required for a docs-only change.

Equivalent Release/x64 solution invocation after discovering MSBuild above:

```powershell
& $msbuild Notepad3.sln /m /p:Configuration=Release /p:Platform=x64 /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw "Build failed: $LASTEXITCODE" }
```

Use the default solution target for integration, as CI does. If a narrower target fails, record its exact command and project context rather than assuming it is universally unsupported. Exit status is required; a fresh `Bin\Release_x64_v145\Notepad3.exe` timestamp alone does not establish a successful build.

## Tests and CI

- For upstream PRs and CI runs, pass `--repo rizonesoft/Notepad3` to `gh` commands. Unqualified commands in a fork checkout can target the fork, returning a missing PR or an unrelated run; confirm the target with `gh repo view --json nameWithOwner`.
- Product CI is `.github\workflows\build.yml`: `windows-2025-vs2026`, Release on Win32, x64, x64_AVX2, and ARM64. Version/GUI tests run on Win32 and x64.
- Docs CI is `.github\workflows\docs.yml`; it runs the existing docs checker independently, including Markdown-only changes that skip the product build.
- Native regression CI is `.github\workflows\native-tests.yml`: one Windows job runs the encoding harness for relevant source, test, fixture, or workflow changes. Docs-only changes do not trigger it; no GUI or full-product coverage is implied.
- `cmd /d /c "call test\TestFileVersion.cmd"` checks existing x86/x64 binaries against today's date and `Versions\build.txt`. Old local binaries can fail without a source regression; compare the reported expected/actual versions.
- The GUI smoke test requires AutoHotkey v2 and an available desktop: `cmd /d /c "cd /d test && call TestAhkNotepad3.cmd"`. It creates `test\_TESTDIR`, copies an executable/config, and writes `test.log` in `test`; do not run it in a read-only review or on a shared desktop without approval.
- In the shared CI `shell: cmd` test step, preserve `|| exit /b 1` after the version test so the GUI command cannot mask its failure.
- The About-box smoke test uses `PostMessage(WM_COMMAND, ...)` rather than `ControlClick`, which was unreliable on the runner desktop. Query dates/versions with PowerShell, not `wmic`.

## Formatting and analysis

`src\.clang-format` and `.editorconfig` are the formatting sources of truth; do not reformat unrelated legacy code. clang-format, clang-tidy, and clangd may be installed under Visual Studio's `VC\Tools\Llvm\x64\bin` even when absent from PATH.

No working repository-wide clang-tidy, compilation-database, or sanitizer recipe is established by this guide. Tool availability alone is not a passing analysis run; report such checks as unrun unless configured and exercised for the affected code.

## Python environment

For scripting tasks, check for the optional git-ignored virtual environment before activating it, and inspect its Python version rather than assuming one:

```powershell
$activate = '.\.venv\Scripts\Activate.ps1'
if (Test-Path -LiteralPath $activate -PathType Leaf) {
    . $activate
    try {
        python --version
        if ($LASTEXITCODE -ne 0) { throw "Python failed: $LASTEXITCODE" }
    } finally {
        deactivate
    }
} else {
    Write-Output 'No .venv; check whether py or python is available.'
}
```

Activate, run Python and deactivate in the same shell call; activation does not persist across separate agent shell calls. If the venv is absent, check `py` / `python` instead; do not require or create a venv. Ask before installing dependencies, and do so only when required by the task.

## Windows platform support

Supported platforms: Win32 (x86), x64, x64_AVX2, and ARM64. ARM 32-bit is not supported; `Release|ARM` maps to Win32.

- For the Notepad3 application, solution `Debug|ARM64` maps to `Debug|Win32` and has no `Build.0` entry; a successful solution build in that configuration does not establish a Debug ARM64 application build ([mapping](../../Notepad3.sln#L384-L389)).
- Both ARM64 and x64 define `_WIN64`; use `_M_ARM64` or `NP3_BUILD_ARM64` in `TypeDefs.h` to distinguish them.
- ARM64 build settings include `CETCompat=false` (CET is x86/x64 only), `TargetMachine=MachineARM64`, and `_WIN64`. Related scripts: `Build\scripts\FixARM64{CETCompat,CrossCompile,OutDir}.ps1`.
