param(
    [string]$DialogsSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$DialogsSource) { $DialogsSource = Join-Path $root 'src\Dialogs.cpp' }
$DialogsSource = (Resolve-Path -LiteralPath $DialogsSource).Path
$options = if ([IO.Path]::GetExtension($DialogsSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-dialogs-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content -LiteralPath $DialogsSource -Raw
    $functions = foreach ($name in @('_InfoBoxLng_GetDefaultBtnId', '_ConvertFilterStrToSpec')) {
        $start = [regex]::Match($source, "(?m)^static int $name\(")
        if (!$start.Success) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start.Index)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
    }
    $selection = Join-Path $temp ('DialogsSelection' + [IO.Path]::GetExtension($DialogsSource))
    $selectionSource = @(
        '#include "TestDialogsSupport.h"',
        '#ifdef __cplusplus', 'extern "C" {', '#endif',
        ($functions -join "`n"),
        'int TestDefaultButton(UINT type) { return _InfoBoxLng_GetDefaultBtnId(type); }',
        'int TestFilterSpec(LPCWSTR filter, COMDLG_FILTERSPEC** spec) { return _ConvertFilterStrToSpec(filter, spec); }',
        '#ifdef __cplusplus', '}', '#endif'
    )
    Set-Content -LiteralPath $selection -Value $selectionSource -Encoding utf8
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $flags = "/nologo /utf-8 /O2 /MT /W4 /WX /DUNICODE /D_UNICODE /I`"$PSScriptRoot`""
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags $options `"$selection`" /Fo:Dialogs.obj && cl $flags /std:c17 `"$PSScriptRoot\TestDialogs.c`" Dialogs.obj user32.lib /Fe:TestDialogs.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Dialogs test build failed' }
    & (Join-Path $temp 'TestDialogs.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Dialogs tests failed' }

    $start = [regex]::Match($source, '(?m)^static void FileMRULookupIcon\(')
    if (!$start.Success) { throw 'Missing FileMRULookupIcon' }
    $body = $source.IndexOf('{', $start.Index)
    $end = [regex]::Match($source.Substring($body), '(?m)^}')
    if (!$end.Success) { throw 'Missing FileMRULookupIcon end' }
    $worker = Join-Path $temp 'MRUIconWorker.cpp'
    Set-Content -LiteralPath $worker -Encoding utf8 -Value @(
        '#include "TestMRUIconSupport.h"', 'extern "C" {',
        ($source.Substring($start.Index, $body + $end.Index + 1 - $start.Index) -replace '^static ', ''), '}'
    )
    $includes = "/I`"$PSScriptRoot`" /I`"$root\src`" /I`"$root\scintilla\include`" /I`"$root\lexilla`" /I`"$root\lexilla\include`""
    $workerFlags = "/nologo /utf-8 /O2 /Gy /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes"
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $workerFlags /std:c++20 /EHa `"$worker`" /Fo:MRUIconWorker.obj && cl /c $workerFlags /std:c++20 /EHa `"$root\src\PathLib.cpp`" /Fo:PathLib.obj && cl /c $workerFlags /std:c++20 /EHa `"$root\src\DynStrg.cpp`" /Fo:DynStrg.obj && cl $workerFlags /std:c17 `"$PSScriptRoot\TestMRUIcon.c`" MRUIconWorker.obj PathLib.obj DynStrg.obj /Fe:TestMRUIcon.exe /link /OPT:REF shell32.lib shlwapi.lib ole32.lib uuid.lib user32.lib comctl32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'MRU icon test build failed' }
    & (Join-Path $temp 'TestMRUIcon.exe')
    if ($LASTEXITCODE -ne 0) { throw 'MRU icon tests failed' }

    $newWindowFunctions = foreach ($signature in @('static bool _PrepareNewWindowArguments(', 'bool DialogNewWindow(')) {
        $start = $source.IndexOf($signature, [StringComparison]::Ordinal)
        if ($start -lt 0) { continue }
        $body = $source.IndexOf('{', $start)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing end of $signature" }
        $source.Substring($start, $body + $end.Index + 1 - $start)
    }
    if (!$source.Contains('bool DialogNewWindow(')) { throw 'Missing DialogNewWindow' }
    Set-Content -LiteralPath (Join-Path $temp 'NewWindow.inc') -Encoding utf8 -Value ($newWindowFunctions -join "`n")
    $notepadSource = Get-Content -LiteralPath (Join-Path $root 'src\Notepad3.cpp') -Raw
    $reload = [regex]::Match($notepadSource, '(?m)^\s*if \(DialogNewWindow\(Globals\.hwndMain, true, Paths\.CurrentFile, NULL\)\) \{\s*CloseApplication\(\);\s*\}')
    if (!$reload.Success) { throw 'Missing conditional new-window reload' }
    Set-Content -LiteralPath (Join-Path $temp 'NewWindowReload.inc') -Encoding utf8 -Value $reload.Value
    $drop = [regex]::Match($notepadSource, '(?m)^\s*if \(DialogNewWindow\(hwnd, sameFile, e->eff, &wi\)\) \{[\s\S]*?^\s*\} else \{\s*_RegisterDropSnapshot\(e->eff\);\s*\}')
    if (!$drop.Success) { throw 'Missing conditional dropped-file launch' }
    Set-Content -LiteralPath (Join-Path $temp 'NewWindowDrop.inc') -Encoding utf8 -Value $drop.Value
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl $workerFlags /std:c++20 /EHsc /I`"$temp`" `"$PSScriptRoot\TestNewWindow.cpp`" /Fe:TestNewWindow.exe /link /OPT:REF user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'New window test build failed' }
    & (Join-Path $temp 'TestNewWindow.exe')
    if ($LASTEXITCODE -ne 0) { throw 'New window tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
