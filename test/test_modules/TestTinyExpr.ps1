param(
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-tinyexpr-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $flags = '/nologo /utf-8 /O2 /MT /W4 /WX /DNDEBUG'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags /std:c++20 /EHs /wd4702 `"$root\src\tinyexprcpp\tinyexpr.cpp`" /Fo:tinyexpr.obj && cl /c $flags /std:c++20 /EHs `"$root\src\tinyexprcpp\np3tinyexpr.cpp`" /Fo:np3tinyexpr.obj && cl $flags /std:c++20 /EHs `"$PSScriptRoot\TestTinyExpr.cpp`" tinyexpr.obj np3tinyexpr.obj /Fe:TestTinyExpr.exe"
    if ($LASTEXITCODE -ne 0) { throw 'TinyExpr test build failed' }
    & (Join-Path $temp 'TestTinyExpr.exe')
    if ($LASTEXITCODE -ne 0) { throw 'TinyExpr tests failed' }

    $edit = Get-Content -LiteralPath (Join-Path $root 'src\Edit.cpp') -Raw
    $start = $edit.IndexOf('#define EXPR_BEG  "${"')
    $end = $edit.IndexOf('//  EditIndentBlock()', $start)
    if ($start -lt 0 -or $end -lt 0) { throw 'Missing EditModifyLines source' }
    $section = $edit.Substring($start, $end - $start)
    Set-Content -LiteralPath (Join-Path $temp 'EditModifyLines.inc') -Encoding utf8 -Value $section
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl $flags /std:c++20 /EHs /DUNICODE /D_UNICODE /I`"$root\src`" /I`"$root\scintilla\include`" /I`"$temp`" `"$PSScriptRoot\TestTinyExprEdit.cpp`" tinyexpr.obj np3tinyexpr.obj /Fe:TestTinyExprEdit.exe"
    if ($LASTEXITCODE -ne 0) { throw 'TinyExpr line-numbering test build failed' }
    & (Join-Path $temp 'TestTinyExprEdit.exe')
    if ($LASTEXITCODE -ne 0) { throw 'TinyExpr line-numbering tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
