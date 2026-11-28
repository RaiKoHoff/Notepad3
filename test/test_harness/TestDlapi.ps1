param(
    [string]$DlapiSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$DlapiSource) { $DlapiSource = Join-Path $root 'src\Dlapi.cpp' }
$DlapiSource = (Resolve-Path -LiteralPath $DlapiSource).Path
$options = if ([IO.Path]::GetExtension($DlapiSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-dlapi-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = "/I`"$root\src`" /I`"$root\scintilla\include`" /I`"$root\lexilla`" /I`"$root\lexilla\include`""
    $flags = "/nologo /utf-8 /O2 /Gy /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes"
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags $options /FI`"$PSScriptRoot\TestDlapiFaults.h`" `"$DlapiSource`" /Fo:Dlapi.obj && cl /c $flags /std:c++20 /EHa `"$root\src\PathLib.cpp`" /Fo:PathLib.obj && cl /c $flags /std:c++20 /EHa `"$root\src\DynStrg.cpp`" /Fo:DynStrg.obj && cl $flags /std:c17 `"$PSScriptRoot\TestDlapi.c`" Dlapi.obj PathLib.obj DynStrg.obj /Fe:TestDlapi.exe /link /OPT:REF shell32.lib shlwapi.lib ole32.lib uuid.lib user32.lib comctl32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'Dlapi test build failed' }
    & (Join-Path $temp 'TestDlapi.exe')
    if ($LASTEXITCODE -ne 0) { throw "Dlapi tests failed: $LASTEXITCODE" }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
