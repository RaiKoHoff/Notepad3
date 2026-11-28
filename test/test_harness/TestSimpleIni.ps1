param(
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Visual C++ build tools not found' }
$vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
$test = Join-Path $PSScriptRoot 'TestSimpleIni.cpp'
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-simpleini-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory -Path $temp)
try {
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /std:c++20 /EHsc /MT /W4 /WX /DUNICODE /D_UNICODE /I`"$root\src\Config`" `"$test`" /Fe:TestSimpleIni.exe"
    if ($LASTEXITCODE -ne 0) { throw 'SimpleIni test build failed' }
    & (Join-Path $temp 'TestSimpleIni.exe') $temp
    if ($LASTEXITCODE -ne 0) { throw 'SimpleIni tests failed' }
} finally {
    Get-ChildItem -LiteralPath $temp -File | Remove-Item
    Remove-Item -LiteralPath $temp
}
