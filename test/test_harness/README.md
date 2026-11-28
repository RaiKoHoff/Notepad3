# Local module regression tests

These tests are for local maintenance and refactoring; native CI does not run them.
They require Windows, PowerShell and Visual C++ build tools.
Generated harness build files use unique directories under the ignored
`test\test_tmp` directory and are removed after each run.

From the repository root, run all tests sequentially (requires approval to use
the shared desktop: the lexer-sorting, zoom-tooltip and Dlapi tests create
hidden Win32 windows):

```powershell
pwsh -NoProfile -File test\test_harness\RunModuleTests.ps1
```

The runner stops at the first failure. It runs the architecture-aware tests on
x86 and x64 and the directory, lexer-sorting and zoom-tooltip regressions on
x64. Individual runners can also be invoked separately; see
[focused native tests](../../agent-trail/build-and-test/focused-native-tests.md)
for their commands, coverage and safety boundaries. When adding a local runner,
add it to `RunModuleTests.ps1` explicitly.
