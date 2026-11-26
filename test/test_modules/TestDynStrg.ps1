param(
    [string]$StringSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$StringSource) { $StringSource = Join-Path $root 'src\DynStrg.cpp' }
$StringSource = (Resolve-Path -LiteralPath $StringSource).Path
$options = if ([IO.Path]::GetExtension($StringSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-dynstrg-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = "/I`"$root\src`" /I`"$root\scintilla\include`""
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /c /utf-8 /O2 /MT /W4 /WX $options /DNDEBUG /DUNICODE /D_UNICODE $includes /FI`"$PSScriptRoot\TestDynStrgFaults.h`" `"$StringSource`" /Fo:DynStrg.obj && cl /nologo /std:c17 /utf-8 /O2 /MT /W4 /WX /DNDEBUG /DUNICODE /D_UNICODE $includes `"$PSScriptRoot\TestDynStrg.c`" DynStrg.obj /Fe:TestDynStrg.exe /link shell32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'DynStrg test build failed' }
    & (Join-Path $temp 'TestDynStrg.exe')
    if ($LASTEXITCODE -ne 0) { throw 'DynStrg tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
