param(
    [string]$DialogsSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$DialogsSource) { $DialogsSource = Join-Path $root 'src\Dialogs.cpp' }
$DialogsSource = (Resolve-Path -LiteralPath $DialogsSource).Path
$options = if ([IO.Path]::GetExtension($DialogsSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-dialogs-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content -LiteralPath $DialogsSource -Raw
    $functions = foreach ($name in @('_InfoBoxLng_GetDefaultBtnId', '_ConvertFilterStrToSpec')) {
        $start = [regex]::Match($source, "(?m)^static int $name\(")
        if (!$start.Success) { throw "Missing $name" }
        $body = $source.IndexOf('{', $start.Index)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing $name end" }
        $source.Substring($start.Index, $body + $end.Index + 1 - $start.Index)
    }
    $selection = Join-Path $temp ('DialogsSelection' + [IO.Path]::GetExtension($DialogsSource))
    $selectionSource = @(
        '#include "TestDialogsSupport.h"',
        '#ifdef __cplusplus', 'extern "C" {', '#endif',
        ($functions -join "`n"),
        'int TestDefaultButton(UINT type) { return _InfoBoxLng_GetDefaultBtnId(type); }',
        'int TestFilterSpec(LPCWSTR filter, COMDLG_FILTERSPEC** spec) { return _ConvertFilterStrToSpec(filter, spec); }',
        '#ifdef __cplusplus', '}', '#endif'
    )
    Set-Content -LiteralPath $selection -Value $selectionSource -Encoding utf8
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $flags = "/nologo /utf-8 /O2 /MT /W4 /WX /DUNICODE /D_UNICODE /I`"$PSScriptRoot`""
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags $options `"$selection`" /Fo:Dialogs.obj && cl $flags /std:c17 `"$PSScriptRoot\TestDialogs.c`" Dialogs.obj user32.lib /Fe:TestDialogs.exe"
    if ($LASTEXITCODE -ne 0) { throw 'Dialogs test build failed' }
    & (Join-Path $temp 'TestDialogs.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Dialogs tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
