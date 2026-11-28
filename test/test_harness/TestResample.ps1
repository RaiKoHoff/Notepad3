param(
    [string]$ResampleSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$ResampleSource) { $ResampleSource = Join-Path $root 'src\Resample.cpp' }
$cppOptions = if ([IO.Path]::GetExtension($ResampleSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-resample-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $test = Join-Path $PSScriptRoot 'TestResample.c'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /c /utf-8 /O2 /MT /W4 /WX $cppOptions /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE /I`"$root\scintilla\include`" `"$ResampleSource`" /Fo:Resample.obj && cl /nologo /utf-8 /O2 /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE /I`"$root\scintilla\include`" `"$test`" Resample.obj /Fe:TestResample.exe /link gdi32.lib user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'Resampler test build failed' }
    & (Join-Path $temp 'TestResample.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Resampler tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
