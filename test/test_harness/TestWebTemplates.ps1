param(
    [string]$Notepad3Source,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$Notepad3Source) { $Notepad3Source = Join-Path $root 'src\Notepad3.cpp' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-webtemplates-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content $Notepad3Source -Raw
    $standardBrowser = $source.Contains('static HRESULT _GetDefaultBrowserClass(std::wstring& browserClass)')
    $standardTarget = $source.Contains('static bool _PrepareWebTemplate(LPCWSTR webTemplate, std::wstring& target)')
    $sharedArgs = $source.Contains('static bool _PrepareBrowserArguments(')
    $standardArgs = $sharedArgs -or $source.Contains('static bool _PrepareWebTemplateArguments(')
    Set-Content (Join-Path $temp 'WebTemplateMode.inc') @("#define WEB_TEMPLATE_STD_BROWSER $([int]$standardBrowser)", "#define WEB_TEMPLATE_STD_TARGET $([int]$standardTarget)", "#define WEB_TEMPLATE_STD_ARGS $([int]$standardArgs)", "#define WEB_TEMPLATE_SHARED_ARGS $([int]$sharedArgs)") -Encoding utf8
    $helpers = foreach ($signature in @('static bool _IsFileWebTemplate(', 'static bool _CanRunWebTemplate(', 'static void _UpdateWebTemplateMenu(', 'static HRESULT _GetDefaultBrowserClass(', 'static DWORD _ExpandWebTemplateEnvironment(', 'static DWORD _AppendEncodedWebTemplateSelection(', 'static bool _FormatSelectionWebTemplate(', 'static bool _PrepareMultiSelectionWebTemplate(', 'static bool _PrepareWebTemplateImpl(', 'static bool _PrepareWebTemplate(', 'static bool _PrepareWebTemplateArguments(', 'static bool _PrepareBrowserArguments(', 'static void _RunWebTemplateAction(')) {
        $start = $source.IndexOf($signature, [StringComparison]::Ordinal)
        if ($start -lt 0) { continue }
        $body = $source.IndexOf('{', $start)
        $end = [regex]::Match($source.Substring($body), '(?m)^}')
        if (!$end.Success) { throw "Missing end of $signature" }
        $source.Substring($start, $body + $end.Index + 1 - $start)
    }
    Set-Content (Join-Path $temp 'WebTemplateHelpers.inc') ($helpers -join "`n") -Encoding utf8

    $pathSource = Get-Content (Join-Path $root 'src\PathLib.cpp') -Raw
    $start = $pathSource.IndexOf('void PTHAPI ExpandEnvironmentStrgs(', [StringComparison]::Ordinal)
    if ($start -lt 0) { throw 'Missing environment expansion helper' }
    $body = $pathSource.IndexOf('{', $start)
    $end = [regex]::Match($pathSource.Substring($body), '(?m)^}')
    if (!$end.Success) { throw 'Missing environment expansion helper end' }
    Set-Content (Join-Path $temp 'WebTemplateEnvironment.inc') $pathSource.Substring($start, $body + $end.Index + 1 - $start) -Encoding utf8

    $start = $source.IndexOf('    case CMD_WEBACTION1:', [StringComparison]::Ordinal)
    if ($start -lt 0) { throw 'Missing WebTemplate command cases' }
    $commands = $source.Substring($start)
    $end = [regex]::Match($commands, '(?m)^(?: {4}| {8})break;')
    if (!$end.Success) { throw 'Missing WebTemplate command end' }
    Set-Content (Join-Path $temp 'WebTemplateCommands.inc') $commands.Substring(0, $end.Index + $end.Length) -Encoding utf8
    $menu = [regex]::Matches($source, '(?m)^    _UpdateWebTemplateMenu\(hmenu,[^\r\n]*')
    if ($menu.Count -ne 1) { throw 'Missing shared WebTemplate menu update' }
    Set-Content (Join-Path $temp 'WebTemplateMenu.inc') ($menu.Value -join "`n") -Encoding utf8
    $hotspot = $source.IndexOf('bool HandleHotSpotURLClicked(', [StringComparison]::Ordinal)
    if ($hotspot -lt 0) { throw 'Missing hyperlink handler' }
    $browser = $source.IndexOf('                if (StrgIsNotEmpty(Settings2.HyperlinkShellExURLWithApp)) {', $hotspot, [StringComparison]::Ordinal)
    if ($browser -lt 0) { throw 'Missing explicit hyperlink browser branch' }
    $end = $source.IndexOf('                } else {', $browser, [StringComparison]::Ordinal)
    if ($end -lt 0) { throw 'Missing explicit hyperlink browser branch end' }
    Set-Content (Join-Path $temp 'HyperlinkBrowser.inc') ($source.Substring($browser, $end - $browser) + '                }') -Encoding utf8
    $config = Get-Content (Join-Path $root 'src\Config\Config.cpp') -Raw
    $start = $config.IndexOf('    IniSectionGetStringNoQuotes(IniSecSettings2, L"WebTemplate1"', [StringComparison]::Ordinal)
    if ($start -lt 0) { throw 'Missing WebTemplate settings block' }
    $end = $config.IndexOf('    Settings2.LexerSQLNumberSignAsComment', $start, [StringComparison]::Ordinal)
    if ($end -lt 0) { throw 'Missing WebTemplate settings block end' }
    Set-Content (Join-Path $temp 'WebTemplateSettings.inc') $config.Substring($start, $end - $start) -Encoding utf8
    $resource = Get-Content (Join-Path $root 'src\Notepad3.rc') -Raw
    $accelerators = [regex]::Match($resource, '(?ms)^IDR_MAINWND ACCELERATORS\r?\nBEGIN.*?^END')
    if (!$accelerators.Success) { throw 'Missing main accelerator table' }
    $menuResource = Join-Path $root 'language\np3_en_us\menu_en_us.rc'
    Set-Content (Join-Path $temp 'WebTemplateResources.rc') @(
        '#include "resource.h"'
        "#include `"$($menuResource.Replace('\', '\\'))`""
        $accelerators.Value
    ) -Encoding utf8

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $test = Join-Path $PSScriptRoot 'TestWebTemplates.cpp'
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && rc /nologo /I`"$root\src`" /fo WebTemplateResources.res WebTemplateResources.rc && cl /nologo /c /std:c++20 /EHa /utf-8 /W4 /WX /DUNICODE /D_UNICODE /I`"$root\scintilla\include`" `"$root\src\DynStrg.cpp`" /Fo:DynStrg.obj && cl /nologo /EHsc /std:c++20 /utf-8 /W4 /WX /DUNICODE /D_UNICODE /I`"$temp`" /I`"$root\scintilla\include`" `"$test`" DynStrg.obj WebTemplateResources.res /Fe:TestWebTemplates.exe /link shlwapi.lib user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'WebTemplate test build failed' }
    & (Join-Path $temp 'TestWebTemplates.exe')
    if ($LASTEXITCODE -ne 0) { throw 'WebTemplate tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
