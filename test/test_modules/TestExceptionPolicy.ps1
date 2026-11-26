param(
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-exceptions-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (!$vs -or !$msbuild) { throw 'Visual C++ build tools or MSBuild not found' }
    $platforms = if ($Architecture -eq 'x86') { @('Win32') } else { @('x64', 'ARM64') }
    foreach ($project in @('src\Notepad3.vcxproj', 'minipath\minipath.vcxproj')) {
        foreach ($platform in $platforms) {
            foreach ($config in @('Debug', 'Release', 'Release_AVX2')) {
                $json = & $msbuild (Join-Path $root $project) "/p:Configuration=$config" "/p:Platform=$platform" /getItem:ClCompile /nologo
                if ($LASTEXITCODE -ne 0) { throw "Cannot evaluate $project $config $platform" }
                $items = ($json | ConvertFrom-Json).Items.ClCompile
                if (!$items -or @($items | Where-Object { $_.ExceptionHandling -ne 'SyncCThrow' }).Count -ne 0) {
                    throw "Inconsistent exception handling: $project $config $platform"
                }
            }
        }
    }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    foreach ($config in @('Debug', 'Release')) {
        $optimization = if ($config -eq 'Debug') { '/Od /MTd /D_DEBUG' } else { '/O2 /MT /DNDEBUG' }
        $flags = "/nologo /utf-8 $optimization /W4 /WX /std:c++20 /EHs"
        & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && cl /c $flags /DTEST_EXCEPTION_THROWER `"$PSScriptRoot\TestExceptionPolicy.cpp`" /Fo:thrower.obj && cl $flags `"$PSScriptRoot\TestExceptionPolicy.cpp`" thrower.obj /Fe:TestExceptionPolicy.exe"
        if ($LASTEXITCODE -ne 0) { throw "$config exception policy test build failed" }
        & (Join-Path $temp 'TestExceptionPolicy.exe')
        if ($LASTEXITCODE -ne 0) { throw "$config exception policy tests failed" }
    }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
