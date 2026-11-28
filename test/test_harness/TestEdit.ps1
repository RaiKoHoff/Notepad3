param(
    [string]$EditSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$EditSource) { $EditSource = Join-Path $root 'src\Edit.cpp' }
$EditSource = (Resolve-Path -LiteralPath $EditSource).Path
$options = if ([IO.Path]::GetExtension($EditSource) -eq '.cpp') { '/std:c++20 /EHs' } else { '/std:c17' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-edit-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content -LiteralPath $EditSource -Raw
    $sortLine = [regex]::Match($source, '(?ms)^typedef struct _SORTLINE \{.*?^\} SORTLINE;')
    if (!$sortLine.Success) { throw 'Missing SORTLINE' }
    $functions = foreach ($name in @('EditReplaceSelection', 'EditDetectEOLMode', '_BumpUInt', '_ParseUInt32', '_ModifyDec', '_ModifyHex', '_ModifyBin', 'CmpStd', 'CmpStdLogical', 'CmpStdLogicalRev')) {
        $start = [regex]::Match($source, "(?m)^(?:static\s+)?(?:unsigned int|void|bool|int)\s+$name\(")
        if (!$start.Success) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start.Index)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
    }
    $selection = Join-Path $temp ('EditSelection' + [IO.Path]::GetExtension($EditSource))
    $selectionSource = @(
        '#include "TestEditSupport.h"',
        '#ifdef __cplusplus', 'extern "C" {', '#endif',
        $sortLine.Value,
        ($functions -join "`n"),
        'bool TestModifyDec(const char* s, bool inc, char* out, size_t size) { return _ModifyDec(s, inc, out, size); }',
        'bool TestModifyHex(const char* s, bool inc, char* out, size_t size) { return _ModifyHex(s, inc, out, size); }',
        'bool TestModifyBin(const char* s, bool inc, char* out, size_t size) { return _ModifyBin(s, inc, out, size); }',
        'int TestLogicalCompare(wchar_t* a, wchar_t* ka, wchar_t* b, wchar_t* kb, bool reverse) { SORTLINE x = { a, ka }, y = { b, kb }; return reverse ? CmpStdLogicalRev(&x, &y) : CmpStdLogical(&x, &y); }',
        '#ifdef __cplusplus', '}', '#endif'
    )
    Set-Content -LiteralPath $selection -Value $selectionSource -Encoding utf8
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $flags = "/nologo /utf-8 /O2 /MT /W4 /WX /DNDEBUG /DUNICODE /D_UNICODE /I`"$PSScriptRoot`" /I`"$root\src`" /I`"$root\scintilla\include`""
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags $options `"$selection`" /Fo:Edit.obj && cl $flags /std:c17 `"$PSScriptRoot\TestEdit.c`" Edit.obj shlwapi.lib /Fe:TestEdit.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Edit test build failed' }
    & (Join-Path $temp 'TestEdit.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Edit tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
