param(
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$tempRoot = Join-Path $root 'test\test_tmp'
[void](New-Item -ItemType Directory -Force -Path $tempRoot)
$temp = Join-Path $tempRoot ('np3-uchardet-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"

    $source = Get-Content -LiteralPath (Join-Path $root 'src\uchardet\EncodingDetection.cpp') -Raw
    $start = $source.IndexOf('cpi_enc_t AnalyzeText_UCHARDET(')
    if ($start -lt 0) { throw 'Missing AnalyzeText_UCHARDET' }
    $end = [regex]::Match($source.Substring($start), '(?m)^}').Index
    if ($end -le 0) { throw 'Missing AnalyzeText_UCHARDET end' }
    $classStart = $source.IndexOf('class AppCharsetDetector final')
    if ($classStart -ge 0 -and $classStart -lt $start) { $start = $classStart; $end = $source.IndexOf("`n}", $source.IndexOf('cpi_enc_t AnalyzeText_UCHARDET(', $start)) + 1 - $start }
    Set-Content -LiteralPath (Join-Path $temp 'UchardetProduction.inc') -Encoding utf8 -Value $source.Substring($start, $end + 1)

    [xml]$project = Get-Content -LiteralPath (Join-Path $root 'src\Notepad3.vcxproj') -Raw
    $sources = @($project.Project.ItemGroup.ClCompile | Where-Object { $_.Include -like 'uchardet\uchardet\src\*.cpp' } |
        ForEach-Object { Join-Path (Join-Path $root 'src') $_.Include })
    $facade = Join-Path $root 'src\uchardet\uchardet\src\uchardet.cpp'
    if ($facade -notin $sources) { $sources += $facade }
    $args = @('/nologo', '/utf-8', '/O2', '/MT', '/W4', '/WX', '/DNDEBUG', '/std:c++20', '/EHs',
        "/I`"$root\src`"", "/I`"$temp`"", "`"$PSScriptRoot\TestUchardet.cpp`"")
    $args += $sources | ForEach-Object { "`"$_`"" }
    $args += '/Fe:TestUchardet.exe'
    Set-Content -LiteralPath (Join-Path $temp 'build.rsp') -Encoding ascii -Value ($args -join "`r`n")
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl @build.rsp"
    if ($LASTEXITCODE -ne 0) { throw 'Uchardet parity test build failed' }
    & (Join-Path $temp 'TestUchardet.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Uchardet parity test failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
