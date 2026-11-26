# Local module regression tests

These tests are for local maintenance and refactoring; native CI does not run them.
They require Windows, PowerShell and Visual C++ build tools.

From the repository root, run all tests sequentially:

```powershell
pwsh -NoProfile -File test\test_modules\RunModuleTests.ps1
```

The runner stops at the first failure. It runs the architecture-aware tests on
x86 and x64 and the two directory regressions on x64. Individual runners can
also be invoked separately; see [focused native tests](../../agent-trail/build-and-test/focused-native-tests.md)
for their commands, coverage and safety boundaries. When adding a local runner,
add it to `RunModuleTests.ps1` explicitly.
