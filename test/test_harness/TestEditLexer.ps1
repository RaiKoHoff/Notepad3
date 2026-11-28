param(
    [string]$LexerSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$LexerSource) { $LexerSource = Join-Path $root 'src\StyleLexers\EditLexer.cpp' }
$options = if ([IO.Path]::GetExtension($LexerSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-edit-lexer-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = "/I`"$root\src`" /I`"$root\scintilla\include`" /I`"$root\lexilla`" /I`"$root\lexilla\include`""
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /c /utf-8 /O2 /MT /W4 /WX $options /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes `"$LexerSource`" /Fo:EditLexer.obj && cl /nologo /std:c17 /utf-8 /O2 /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes `"$PSScriptRoot\TestEditLexer.c`" EditLexer.obj /Fe:TestEditLexer.exe"
    if ($LASTEXITCODE -ne 0) { throw 'EditLexer test build failed' }
    & (Join-Path $temp 'TestEditLexer.exe')
    if ($LASTEXITCODE -ne 0) { throw 'EditLexer tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
