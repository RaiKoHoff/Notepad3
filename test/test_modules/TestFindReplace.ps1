param(
    [string]$FindReplaceSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$FindReplaceSource) { $FindReplaceSource = Join-Path $root 'src\FindReplace.cpp' }
$FindReplaceSource = (Resolve-Path -LiteralPath $FindReplaceSource).Path
$options = if ([IO.Path]::GetExtension($FindReplaceSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-findreplace-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content -LiteralPath $FindReplaceSource -Raw
    $mode = [regex]::Match($source, '(?m)^typedef enum \{ FRMOD_IGNORE[^\r\n]+')
    if (!$mode.Success) { throw 'Missing search update modes' }
    $functions = foreach ($name in @('_FindInTarget', '_UnSlashLowOctalA', '_CheckRegExReplTargetA', '_TransformReplaceBackslashes', '_EscapeWildcards')) {
        $start = [regex]::Match($source, "(?m)^static\s+(?:DocPos|void|int)\s+$name\([^;]*?\)\s*\{")
        if (!$start.Success) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start.Index)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
    }
    $selection = Join-Path $temp ('FindReplaceSelection' + [IO.Path]::GetExtension($FindReplaceSource))
    $selectionSource = @(
        '#include "TestFindReplaceSupport.h"',
        '#ifdef __cplusplus', 'extern "C" {', '#endif',
        '#define NOT_FOUND ((DocPos)(-1LL))', $mode.Value, ($functions -join "`n"),
        'DocPos TestFindInTarget(LPCWSTR text, int flags, DocPos* begin, DocPos* end, bool force, int mode) { return _FindInTarget(text, flags, begin, end, force, (FR_UPD_MODES)mode); }',
        'void TestTransformReplacement(LPSTR text, bool regex, int* message) { _TransformReplaceBackslashes(text, regex, message); }',
        'void TestEscapeWildcards(LPWSTR text, size_t capacity, LPEDITFINDREPLACE data) { _EscapeWildcards(text, capacity, data); }',
        '#ifdef __cplusplus', '}', '#endif'
    )
    Set-Content -LiteralPath $selection -Value $selectionSource -Encoding utf8
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = "/I`"$root\src`" /I`"$PSScriptRoot`" /I`"$root\scintilla\include`" /I`"$root\lexilla`" /I`"$root\lexilla\include`""
    $flags = "/nologo /utf-8 /O2 /Gy /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes"
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags $options `"$selection`" /Fo:FindReplace.obj && cl /c $flags /std:c++20 /EHa `"$root\src\Helpers.cpp`" /Fo:Helpers.obj && cl /c $flags /std:c++20 /EHa `"$root\src\PathLib.cpp`" /Fo:PathLib.obj && cl /c $flags /std:c++20 /EHa `"$root\src\DynStrg.cpp`" /Fo:DynStrg.obj && cl $flags /std:c17 `"$PSScriptRoot\TestFindReplace.c`" FindReplace.obj Helpers.obj PathLib.obj DynStrg.obj /Fe:TestFindReplace.exe /link /OPT:REF /NODEFAULTLIB:user32-stub.lib /NODEFAULTLIB:uxtheme-stub.lib shell32.lib shlwapi.lib ole32.lib uuid.lib user32.lib advapi32.lib gdi32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'FindReplace test build failed' }
    & (Join-Path $temp 'TestFindReplace.exe')
    if ($LASTEXITCODE -ne 0) { throw 'FindReplace tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
