param(
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$source = Get-Content -LiteralPath (Join-Path $root 'src\Notepad3.cpp') -Raw
$state = [regex]::Match($source, '(?ms)^struct FileWatchRun \{.*?^\} s_fileWatch = [^\r\n]+;')
if (!$state.Success) { throw 'Missing watcher state' }
$functions = foreach ($name in @('IsFileChangedFlagSet', 'IsFileDeletedFlagSet', 'ProbeFileWatch', 'SetFileObservationData', 'ReleaseFileWatchRun', 'StopFileWatch', 'IsStaleFileWatchMessage', 'PostFileWatchChange', 'HandleFileWatchFailure', 'FileChangeObserver')) {
    $start = [regex]::Match($source, "(?m)^(?:static inline \w+|static LRESULT|static unsigned int WINAPI)\s+$name\([^\r\n]*\)\s*(?=\{)")
    if (!$start.Success) { throw "Missing $name" }
    $body = $source.IndexOf('{', $start.Index)
    $end = [regex]::Match($source.Substring($body), '(?m)^}')
    if (!$end.Success) { throw "Missing $name end" }
    $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
}
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-watcher-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $test = Join-Path $temp 'TestFileWatcher.cpp'
    Set-Content -LiteralPath $test -Encoding utf8 -Value @(
        '#include "TestFileWatcherSupport.h"',
        $state.Value,
        ($functions -join "`n"),
        '#include "TestFileWatcherCases.h"'
    )
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /utf-8 /O2 /MT /W4 /WX /std:c++20 /EHs /DUNICODE /D_UNICODE /I`"$PSScriptRoot`" `"$test`" /Fe:TestFileWatcher.exe"
    if ($LASTEXITCODE -ne 0) { throw 'File watcher test build failed' }
    & (Join-Path $temp 'TestFileWatcher.exe')
    if ($LASTEXITCODE -ne 0) { throw 'File watcher tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
