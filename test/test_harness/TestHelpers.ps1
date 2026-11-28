param(
    [string]$HelpersSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$HelpersSource) { $HelpersSource = Join-Path $root 'src\Helpers.cpp' }
$HelpersSource = (Resolve-Path -LiteralPath $HelpersSource).Path
$options = if ([IO.Path]::GetExtension($HelpersSource) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-helpers-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = "/I`"$root\src`" /I`"$root\scintilla\include`" /I`"$root\lexilla`" /I`"$root\lexilla\include`""
    $flags = "/nologo /utf-8 /O2 /Gy /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes"
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags $options /FI`"$PSScriptRoot\TestHelpersFaults.h`" `"$HelpersSource`" /Fo:Helpers.obj && cl /c $flags /std:c++20 /EHa `"$root\src\PathLib.cpp`" /Fo:PathLib.obj && cl /c $flags /std:c++20 /EHa /FI`"$PSScriptRoot\TestDynStrgFaults.h`" `"$root\src\DynStrg.cpp`" /Fo:DynStrg.obj && cl /c $flags /std:c++20 /EHsc `"$PSScriptRoot\TestHelpersAlloc.cpp`" /Fo:TestHelpersAlloc.obj && cl $flags /std:c17 `"$PSScriptRoot\TestHelpers.c`" TestHelpersAlloc.obj Helpers.obj PathLib.obj DynStrg.obj /Fe:TestHelpers.exe /link /OPT:REF /NODEFAULTLIB:user32-stub.lib /NODEFAULTLIB:uxtheme-stub.lib shell32.lib shlwapi.lib ole32.lib uuid.lib user32.lib advapi32.lib gdi32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'Helpers test build failed' }
    & (Join-Path $temp 'TestHelpers.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Helpers tests failed' }

    $source = Get-Content -LiteralPath $HelpersSource -Raw
    $start = $source.IndexOf('HRESULT SetWindowAppUserModelID(', [StringComparison]::Ordinal)
    if ($start -lt 0) { throw 'Missing SetWindowAppUserModelID' }
    $body = $source.IndexOf('{', $start)
    $end = [regex]::Match($source.Substring($body), '(?m)^}')
    if (!$end.Success) { throw 'Missing SetWindowAppUserModelID end' }
    Set-Content -LiteralPath (Join-Path $temp 'AppUserModel.inc') -Encoding utf8 -Value $source.Substring($start, $body + $end.Index + 1 - $start)
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl $flags /std:c++20 /EHsc /I`"$temp`" `"$PSScriptRoot\TestAppUserModel.cpp`" /Fe:TestAppUserModel.exe /link /OPT:REF ole32.lib uuid.lib user32.lib"
    if ($LASTEXITCODE -ne 0) { throw 'AppUserModel test build failed' }
    & (Join-Path $temp 'TestAppUserModel.exe')
    if ($LASTEXITCODE -ne 0) { throw 'AppUserModel tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
