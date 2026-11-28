$ErrorActionPreference = 'Stop'

$architectureTests = @(
    'TestResample.ps1'
    'TestLexerLayout.ps1'
    'TestEditLexer.ps1'
    'TestEncodingUnit.ps1'
    'TestMuiLanguage.ps1'
    'TestDynStrg.ps1'
    'TestPathLib.ps1'
    'TestHelpers.ps1'
    'TestIniFileLock.ps1'
    'TestSimpleIni.ps1'
    'TestDlapi.ps1'
    'TestStyles.ps1'
    'TestFindReplace.ps1'
    'TestDialogs.ps1'
    'TestEdit.ps1'
    'TestNotepad3.ps1'
    'TestWebTemplates.ps1'
    'TestFileWatcher.ps1'
    'TestTinyExpr.ps1'
    'TestUchardet.ps1'
)

foreach ($test in $architectureTests) {
    foreach ($architecture in @('x86', 'x64')) {
        Write-Host "`n=== $test ($architecture) ==="
        try {
            & (Join-Path $PSScriptRoot $test) -Architecture $architecture
        } catch {
            throw "$test ($architecture) failed: $_"
        }
    }
}

foreach ($test in @('TestOpenDialogDirectory.ps1', 'TestCreateDirectory.ps1', 'TestLexerDialogSorting.ps1', 'TestZoomTooltipPosition.ps1')) {
    Write-Host "`n=== $test (x64) ==="
    try {
        & (Join-Path $PSScriptRoot $test)
    } catch {
        throw "$test (x64) failed: $_"
    }
}

Write-Host "`nAll module regression tests passed."
