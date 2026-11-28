param(
    [string]$EncodingSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$EncodingSource) { $EncodingSource = Join-Path $root 'src\Encoding.cpp' }
$EncodingSource = (Resolve-Path -LiteralPath $EncodingSource).Path
$options = if ([IO.Path]::GetExtension($EncodingSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-encoding-unit-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = "/I`"$root\src`" /I`"$root\scintilla\include`" /I`"$root\lexilla`" /I`"$root\lexilla\include`""
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /c /utf-8 /O2 /Gy /MT /W4 /WX $options /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes `"$EncodingSource`" /Fo:Encoding.obj && cl /nologo /std:c17 /utf-8 /O2 /Gy /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes `"$PSScriptRoot\TestEncodingUnit.c`" Encoding.obj /Fe:TestEncodingUnit.exe /link /OPT:REF user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'Encoding unit test build failed' }
    & (Join-Path $temp 'TestEncodingUnit.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Encoding unit tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
