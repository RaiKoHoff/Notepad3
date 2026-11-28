param(
    [string]$LanguageSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$LanguageSource) { $LanguageSource = Join-Path $root 'src\MuiLanguage.cpp' }
$LanguageSource = (Resolve-Path -LiteralPath $LanguageSource).Path
$options = if ([IO.Path]::GetExtension($LanguageSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-muilanguage-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = "/I`"$root\src`" /I`"$root\scintilla\include`" /I`"$root\lexilla`" /I`"$root\lexilla\include`""
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && rc /nologo /fo TestMuiLanguage.res `"$PSScriptRoot\TestMuiLanguage.rc`" && cl /nologo /c /utf-8 /O2 /Gy /MT /W4 /WX $options /DNDEBUG /DMUI_BASE_LNG_EN_US /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes `"$LanguageSource`" /Fo:MuiLanguage.obj && cl /nologo /std:c17 /utf-8 /O2 /Gy /MT /W4 /WX /DNDEBUG /DMUI_BASE_LNG_EN_US /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes `"$PSScriptRoot\TestMuiLanguage.c`" MuiLanguage.obj TestMuiLanguage.res /Fe:TestMuiLanguage.exe /link /OPT:REF user32.lib advapi32.lib comctl32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'MuiLanguage test build failed' }
    & (Join-Path $temp 'TestMuiLanguage.exe')
    if ($LASTEXITCODE -ne 0) { throw 'MuiLanguage tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
