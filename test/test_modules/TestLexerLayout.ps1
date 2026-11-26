param(
    [string]$LexerSource,
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (!$LexerSource) { $LexerSource = Join-Path $root 'src\StyleLexers\styleLexCPP.cpp' }
$lexerSources = @(
    $LexerSource,
    (Join-Path $root 'src\StyleLexers\styleLexJAVA.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexJS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexCS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexRC.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexD.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexGo.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexPY.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexAwk.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexJSON.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexJSON5.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexYAML.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexTOML.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexVB.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexVBS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexBASH.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexBAT.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexPS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexRUBY.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexPL.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexLUA.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexR.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexMATLAB.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexRust.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexKotlin.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexDart.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexJulia.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexPAS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexFortran.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexVHDL.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexVerilog.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexCMAKE.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexMAK.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexCONF.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexPROPS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexAHK.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexAHK2.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexCSS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexMARKDOWN.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexINNO.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexNSIS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexAbaqus.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexAPDL.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexCSV.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexDIFF.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexASM.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexRegistry.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexAU3.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexKiX.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexAVS.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexCOFFEESCRIPT.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexNim.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexTCL.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexLaTex.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexSQL.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexHTML.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexXML.cpp'),
    (Join-Path $root 'src\StyleLexers\styleLexStandard.cpp')
)
$temp = Join-Path ([IO.Path]::GetTempPath()) ('np3-lexer-layout-' + [guid]::NewGuid())
[void](New-Item -ItemType Directory $temp)
try {
    $source = Get-Content (Join-Path $root 'src\Styles.cpp') -Raw
    $start = $source.IndexOf('void Style_FillRelatedStyles(', [StringComparison]::Ordinal)
    if ($start -lt 0) { throw 'Missing Style_FillRelatedStyles' }
    $body = $source.IndexOf('{', $start)
    $end = [regex]::Match($source.Substring($body), '(?m)^}')
    if (!$end.Success) { throw 'Missing Style_FillRelatedStyles end' }
    Set-Content (Join-Path $temp 'StyleFillRelatedStyles.inc') $source.Substring($start, $body + $end.Index + 1 - $start) -Encoding utf8

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual C++ build tools not found' }
    $vcvars = Join-Path $vs "VC\Auxiliary\Build\vcvars$($Architecture.Replace('x86', '32').Replace('x64', '64')).bat"
    $includes = '/I"%~2\src" /I"%~2\scintilla\include" /I"%~2\lexilla" /I"%~2\lexilla\include"'
    $compileCommands = for ($i = 0; $i -lt $lexerSources.Count; ++$i) {
        $options = if ([IO.Path]::GetExtension($lexerSources[$i]) -eq '.cpp') { '/std:c++20 /EHa' } else { '/std:c17' }
        $sourceArg = if ($i -eq 0) { '%~1' } else { '%~2' + $lexerSources[$i].Substring($root.Length) }
        "cl /nologo /c /utf-8 /O2 /MT /W4 /WX $options /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes `"$sourceArg`" /Fo:Lexer$i.obj"
    }
    $objects = (0..($lexerSources.Count - 1) | ForEach-Object { "Lexer$_.obj" }) -join ' '
    $buildScript = Join-Path $temp 'BuildLexerLayout.cmd'
    $buildCommands = @('@echo off') + @($compileCommands | ForEach-Object { "$_ || exit /b 1" })
    $buildCommands += "cl /nologo /std:c17 /utf-8 /O2 /MT /W4 /WX /DNDEBUG /DD_NP3_WIN10_DARK_MODE /DUNICODE /D_UNICODE $includes /I`".`" `"%~2\test\test_modules\TestLexerLayout.c`" $objects /Fe:TestLexerLayout.exe"
    Set-Content -LiteralPath $buildScript -Value $buildCommands -Encoding utf8
    & $env:ComSpec /d /c "call `"$vcvars`" >nul && cd /d `"$temp`" && call `"$buildScript`" `"$LexerSource`" `"$root`""
    if ($LASTEXITCODE -ne 0) { throw 'Lexer-layout test build failed' }
    & (Join-Path $temp 'TestLexerLayout.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Lexer-layout tests failed' }
} finally {
    Get-ChildItem $temp -File | Remove-Item
    Remove-Item $temp
}
