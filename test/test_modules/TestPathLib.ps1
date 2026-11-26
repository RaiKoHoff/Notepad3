param(
    [string]$PathLibSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$PathLibSource) { $PathLibSource = Join-Path $root 'src\PathLib.cpp' }
$PathLibSource = (Resolve-Path -LiteralPath $PathLibSource).Path
$options = if ([IO.Path]::GetExtension($PathLibSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-pathlib-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = "/I`"$root\src`" /I`"$root\scintilla\include`" /I`"$root\lexilla`" /I`"$root\lexilla\include`""
    $flags = "/nologo /utf-8 /O2 /Gy /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes"
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags $options `"$PathLibSource`" /Fo:PathLib.obj && cl /c $flags /std:c++20 /EHa `"$root\src\DynStrg.cpp`" /Fo:DynStrg.obj && cl /c $flags /std:c++20 /EHa `"$PSScriptRoot\TestPathLibCpp.cpp`" /Fo:TestPathLibCpp.obj && cl $flags /std:c17 `"$PSScriptRoot\TestPathLib.c`" PathLib.obj DynStrg.obj TestPathLibCpp.obj /Fe:TestPathLib.exe /link /OPT:REF shell32.lib shlwapi.lib ole32.lib uuid.lib user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'PathLib test build failed' }
    & (Join-Path $temp 'TestPathLib.exe')
    if ($LASTEXITCODE -ne 0) { throw 'PathLib tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
