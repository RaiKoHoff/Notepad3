param(
    [string]$Notepad3Source,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$Notepad3Source) { $Notepad3Source = Join-Path $root 'src\Notepad3.cpp' }
$Notepad3Source = (Resolve-Path -LiteralPath $Notepad3Source).Path
$options = if ([IO.Path]::GetExtension($Notepad3Source) -eq '.cpp') { '/std:c++20 /EHs' } else { '/std:c17' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-main-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content -LiteralPath $Notepad3Source -Raw
    $mode = [regex]::Match($source, '(?ms)^typedef enum TE_OUT_MODE_T \{.*?^\} TE_OUT_MODE_T;')
    if (!$mode.Success) { throw 'Missing TinyExpr output mode' }
    $constants = foreach ($name in @('TE_ZERO', 'TE_FMTA', 'TE_FMTW', 'TE_INT64_DBL_MAX', 'TE_INT32_DBL_MAX')) {
        $definition = [regex]::Match($source, "(?m)^#define $name\s+[^\r\n]+")
        if (!$definition.Success) { throw "Missing $name" }
        $definition.Value
    }
    $functions = foreach ($name in @('_TinyExprBitWidth', '_TinyExprIntBound', '_FormatBinA', '_FormatBinW', 'TinyExprToStringA', 'TinyExprToString')) {
        $start = [regex]::Match($source, "(?m)^(?:static\s+)?(?:int|double|void)\s+$name\(")
        if (!$start.Success) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start.Index)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
    }
    $extracted = Join-Path $temp ('Notepad3Formatting' + [IO.Path]::GetExtension($Notepad3Source))
    Set-Content -LiteralPath $extracted -Encoding utf8 -Value @(
        '#include "TestNotepad3Support.h"',
        '#ifdef __cplusplus', 'extern "C" {', '#endif',
        $mode.Value,
        'static TE_OUT_MODE_T s_iTinyExprOutMode = TE_OUT_DEC;',
        ($constants -join "`n"),
        ($functions -join "`n"),
        'void TestSetTinyExprMode(int mode) { s_iTinyExprOutMode = (TE_OUT_MODE_T)mode; }',
        '#ifdef __cplusplus', '}', '#endif'
    )
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $flags = "/nologo /utf-8 /O2 /MT /W4 /WX /DNDEBUG /DUNICODE /D_UNICODE /I`"$PSScriptRoot`""
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags $options `"$extracted`" /Fo:Notepad3.obj && cl $flags /std:c17 `"$PSScriptRoot\TestNotepad3.c`" Notepad3.obj /Fe:TestNotepad3.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Notepad3 test build failed' }
    & (Join-Path $temp 'TestNotepad3.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Notepad3 tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
