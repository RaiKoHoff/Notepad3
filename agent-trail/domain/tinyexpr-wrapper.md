# TinyExpr++ and the Notepad3 Interface

When to read: Expression evaluation (`=?`, Go To, line numbering), `np3tinyexpr`

Notepad3 callers use the typed interface in `src\tinyexprcpp\np3tinyexpr.h` / `np3tinyexpr.cpp`; they do not include `src\tinyexprcpp\tinyexpr.h` directly. Do not edit the vendored `tinyexpr.cpp` / `tinyexpr.h` to fix Notepad3 behavior; adapt the interface. Library version is recorded in `src\tinyexprcpp\version.txt` and `TINYEXPR_CPP_*_VERSION`.

The typed core returns evaluation value, 1-based error and logical classification together. Its compiled expression is move-only; binding names are copied, but bound values must outlive the compiled expression. Disabling classification avoids work when it is not needed. Core operations report failure without propagating exceptions.

## Interface contract (`np3tinyexpr.h`)

| Function | Notes |
|---|---|
| `evaluate(expr, classify)` | Parse + evaluate. NaN on error; error is `0` or the **1-based** position, clamped to the input length. Classification can be disabled. |
| `Expression::compile` / `evaluate` | Bound variables are `{name, const double*}`; pointers must outlive the compiled expression. Compilation fails for a bad expression, null name/address, or duplicate name. |
| `is_logical(expr)` | Lexical check (depth 0 operators, `true`/`false`, `AND`/`OR`/`NOT`/`IS*` calls); never evaluates. |
| `supports_64bit` | `false` for the default `double` te_type. |

## Invariants in `np3tinyexpr.cpp`

- **`te_cif_preprocess()` keeps the input length.** TinyExpr++ error positions index the text it was given, so every rewrite must be length-preserving, otherwise callers (`Edit.cpp` truncating at `err`, Go To dialog) point to the wrong character. It blanks a leading `=` (spreadsheet prefix the library would strip) and terminated comments (a comment acts as whitespace, so `1/**/2` is a parse error rather than `12`), rewrites `0b<bits>` to decimal padded with blanks (saturates above 64 significant bits), and copies `"..."` literals verbatim. An unterminated `/*` is left for the library to report.
- **String literals** have no escapes in the vendored library version; the next `"` ends them. `te_strip_outer_parens` and `te_expression_is_logical` must skip them.
- **`0b` token boundaries** are operators, `(`, `,`, `;`, whitespace, `~`, and the cp1252 `×` / `÷` (`0xD7` / `0xF7`).
- **`te_cif_init_parser()` registers the compat `log` (log10) and the locale separators.** In `Expression::compile` call it *after* `set_variables_and_functions()`, which replaces the whole set. A user variable named `log` takes precedence (`insert` does not overwrite).
- **Locale separators:** if the C runtime locale decimal point is `,`, decimal becomes `,` and list separator `;`.
- Interface operations catch C++ exceptions; none may escape into editor callbacks. Structured exceptions are not caught by C++ `catch (...)` under the application's `/EHs` setting.

## Callers

`Notepad3.cpp` `_EvalTinyExpr` (`=?` trigger), the status-bar selection evaluator and `_InterpMultiSelectionTinyExpr`, `Edit.cpp` `EditModifyNumber`, `EditModifyLinesDlg` (compile once, evaluate per line with `L`, `I`, `N`), Go To dialog, `Dialogs.cpp` autosave interval, `Styles.cpp` dark-mode highlight contrast. Results converted to line/column numbers go through `d2lnc()` (`Helpers.h`; NaN -> 0, out-of-range and Inf clamped, unlike `d2ln`) in `EditModifyLinesDlg` and the Go To dialog; `EditModifyNumber` evaluates `"(%s\n)%s"` with `+1` or `-1`, so a trailing `//` comment cannot swallow the closing parenthesis. Inline input is converted with codepage 1252 and `#` as the replacement character.

## Test

The local `test\test_harness\TestTinyExpr.ps1` runner links the real interface and library to C++ tests on x86 or x64. It covers preprocessing, error offsets, logical classification, compiled-variable bindings, move ownership and comma-decimal separators; it reports a skip if a comma-decimal locale is unavailable. It also exercises extracted line-numbering code with a stub document and undo callbacks, including a raised structured exception; it does not exercise actual Scintilla or GUI behavior.
