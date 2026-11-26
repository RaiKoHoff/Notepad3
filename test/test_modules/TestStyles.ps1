param(
    [string]$StylesSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$StylesSource) { $StylesSource = Join-Path $root 'src\Styles.cpp' }
$StylesSource = (Resolve-Path -LiteralPath $StylesSource).Path
$options = if ([IO.Path]::GetExtension($StylesSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-styles-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content -LiteralPath $StylesSource -Raw
    $registry = [regex]::Match($source, '(?ms)^static PEDITLEXER g_pLexArray\[\] = \{.*?^\};')
    if (!$registry.Success) { throw 'Missing lexer registry' }
    $functions = foreach ($name in @('Style_StrHasAttributeEx', 'Style_StrHasAttribute', 'Style_SniffShebang', 'Style_MatchLexer', 'Style_WildcardMatchLexer', '_style_copy', '_style_dtor')) {
        $start = [regex]::Match($source, "(?m)^(?:static\s+)?(?:__forceinline\s+)?(?:bool|void|PEDITLEXER)\s+$name\(")
        if (!$start.Success) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start.Index)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
    }
    $selection = Join-Path $temp ('StyleSelection' + [IO.Path]::GetExtension($StylesSource))
    $selectionSource = @(
        '#ifdef __cplusplus', 'extern "C" {', '#endif',
        '#include "Helpers.h"', '#include "Styles.h"',
        $registry.Value, ($functions -join "`n"),
        'LPWSTR Style_TestCopy(LPCWSTR input) { LPWSTR output = NULL; _style_copy(&output, &input); return output; }',
        'void Style_TestFree(LPWSTR* value) { _style_dtor(value); }',
        '#ifdef __cplusplus', '}', '#endif'
    )
    Set-Content -LiteralPath $selection -Value $selectionSource -Encoding utf8
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = '/I"%~2\src" /I"%~2\scintilla\include" /I"%~2\lexilla" /I"%~2\lexilla\include"'
    $flags = "/nologo /utf-8 /O2 /Gy /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes"
    $lexerSources = @(Get-ChildItem -LiteralPath (Join-Path $root 'src\StyleLexers') -Filter 'styleLex*.cpp' -File)
    $commands = @('@echo off', "cl /c $flags $options `"%~1`" /Fo:Styles.obj || exit /b 1")
    $objects = @('Styles.obj')
    for ($i = 0; $i -lt $lexerSources.Count; ++$i) {
        $object = "Lexer$i.obj"
        $relative = $lexerSources[$i].FullName.Substring($root.Length)
        $commands += "cl /c $flags /std:c++20 /EHa `"%~2$relative`" /Fo:$object || exit /b 1"
        $objects += $object
    }
    $commands += "cl $flags /std:c17 `"%~2\test\test_modules\TestStyles.c`" $($objects -join ' ') /Fe:TestStyles.exe /link /OPT:REF /NODEFAULTLIB:user32-stub.lib /NODEFAULTLIB:uxtheme-stub.lib shlwapi.lib"
    $buildScript = Join-Path $temp 'BuildStyles.cmd'
    Set-Content -LiteralPath $buildScript -Value $commands -Encoding utf8
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && call `"$buildScript`" `"$selection`" `"$root`""
    if ($LASTEXITCODE -ne 0) { throw 'Styles test build failed' }
    & (Join-Path $temp 'TestStyles.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Styles tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
