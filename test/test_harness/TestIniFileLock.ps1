param(
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$source = Get-Content -LiteralPath (Join-Path $root 'src\Config\Config.cpp') -Raw
$start = $source.IndexOf('static void _ReportIniLockError(', [StringComparison]::Ordinal)
$end = $source.IndexOf('bool ReleaseFileLock(', [StringComparison]::Ordinal)
if ($start -lt 0 -or $end -le $start) { throw 'Missing INI lock acquisition code' }

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Visual C++ build tools not found' }
$vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
$test = Join-Path $PSScriptRoot 'TestIniFileLock.cpp'
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-ini-lock-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory -Path $temp)
try {
    Set-Content -LiteralPath (Join-Path $temp 'IniFileLock.inc') -Encoding utf8 -Value $source.Substring($start, $end - $start)
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /nologo /std:c++20 /EHsc /MT /W4 /WX /DUNICODE /D_UNICODE /I`"$temp`" `"$test`" /Fe:TestIniFileLock.exe"
    if ($LASTEXITCODE -ne 0) { throw 'INI lock test build failed' }
    & (Join-Path $temp 'TestIniFileLock.exe')
    if ($LASTEXITCODE -ne 0) { throw 'INI lock tests failed' }
} finally {
    Get-ChildItem -LiteralPath $temp -File | Remove-Item
    Remove-Item -LiteralPath $temp
}
