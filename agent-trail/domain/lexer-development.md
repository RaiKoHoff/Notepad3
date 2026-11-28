# Lexer Development

When to read: Lexers, styles, themes

Each language has an `EDITLEXER` definition in `src\StyleLexers\styleLex<name>.c` or `.cpp`. Register new lexers in the `Styles.cpp` lexer array, declare `extern EDITLEXER lex<name>;` in `src\StyleLexers\EditLexer.h`, add the source to `src\Notepad3.vcxproj` and its `.filters` file, and add localization string IDs.

When inventorying definitions, `lexF77` is inside `#if 0` and is not registered; a textual definition count includes it ([source](../../src/StyleLexers/styleLexFortran.cpp#L104-L154), [registry](../../src/Styles.cpp#L61-L124)).

## Implementation touchpoints

- **C++ conversion:** `EDITLEXER` ends in `Styles[]`, initialized by each lexer definition; preserve its layout, sentinel and mixed-language linkage when converting. Include `StyleLexers.h` inside the `extern "C"` block so the existing lexer declarations have C linkage too; wrapping only the definitions caused unresolved C symbols in the lexer-layout harness. Verify the proposed representation with the actual C++ compiler before bulk conversion ([definition](../../src/StyleLexers/EditLexer.h#L35-L45), [example](../../src/StyleLexers/styleLexCPP.cpp#L46-L72)).
- **Scintilla properties:** if a lexer exposes properties (for example, `lexer.foo.allow.X`), add its `case SCLEX_FOO:` to the property dispatch in `src\StyleLexers\EditLexer.cpp` and call `SciCall_SetProperty(...)`. Otherwise constructor defaults silently remain in effect.
- **Shared lexer IDs:** `SCLEX_CPP` is used for C, C#, Java, JS, and RC; `SCLEX_D` for D and Go; `SCLEX_PYTHON` for Python and Awk. Changes to a shared lexer or its property dispatch can affect every language wired to that ID.
- **HTML/XML style dispatch:** `Style_FillRelatedStyles` expands default rows to related embedded-language styles. XML CDATA also applies to PHP, JavaScript, VB and Python styles; its Python and ASP-Python styles are dispatched twice by the shared and XML-only branches. Preserve or explicitly review that order, and characterize the calls, not only the style rows ([source](../../src/Styles.cpp#L2201-L2280), [stubbed test](../../test/test_harness/TestLexerLayout.c#L143-L154)).
- **SQL comment dispatch:** The Comment row applies to related comment styles first; later Doc Comment and Doc Comment Keyword rows reapply their respective styles. Preserve this order ([dispatch](../../src/Styles.cpp#L2290-L2294), [rows](../../src/StyleLexers/styleLexSQL.cpp#L106-L115), [stubbed test](../../test/test_harness/TestLexerLayout.c#L546-L552)).
- **CSV extra spacing:** Its extra-spacing row is dispatched as style IDs 188 and 19 by `Style_FillRelatedStyles`; preserve or explicitly review that behavior when changing the row. The lexer-layout harness characterizes the calls with rendering stubbed; actual Scintilla impact is unverified ([row](../../src/StyleLexers/styleLexCSV.cpp#L14), [dispatch](../../src/Styles.cpp#L2192-L2199)).
- **Comment toggling:** lexers with comments need `SCLEX_FOO` cases in both `Lexer_GetStreamCommentStrgs` and `Lexer_GetLineCommentStrg` in `src\StyleLexers\EditLexer.cpp`.
- **Line-comment fallthrough:** `SCLEX_NULL`, CSS, diff and Markdown also enter the HTML style-range checks; changing this fallthrough can change their returned delimiter for matching style IDs ([dispatch](../../src/StyleLexers/EditLexer.cpp#L165-L180), [stubbed test](../../test/test_harness/TestEditLexer.c#L136-L138)).
- **Theme sections:** INI sections use the `pszName` field (the fourth `EDITLEXER` field), not the lexer name string. Add sections for new lexers to applicable theme INIs: `Build\Notepad3.ini`, `Build\Themes\*.ini`, `res\StdDarkModeScheme.ini`, and the portable-package copies `Build\Notepad3_pap.ini`, `Build\Notepad3_ptb_x64.ini` and `Build\Notepad3_ptb_x86.ini`. Renaming `pszName` can orphan existing user style customizations.
- **New style rows:** add a matching style entry to the applicable theme overrides. Only `res\StdDarkModeScheme.ini` and `Build\Themes\{Dark,Obsidian,Sombra}.ini` have per-style overrides; the main and portable INIs often have empty lexer sections where inline defaults apply. For foreground/background pairs, target WCAG contrast of at least 4.5:1.
- **Style localization:** each new `EDITSTYLE` label needs an `IDS_LEX_STR_*` definition in `language\common_res.h` and an English placeholder in all 26 `language\np3_*\lexer_*.rc` files. Check ID gaps when choosing an ID. For bulk insertion, anchor on an existing identifier (not translated text), run once, and verify one occurrence per file.

## Lexilla sources

Homebrew lexer sources under `lexilla\lexers_x\` include AutoHotkey (`LexAHK` and `LexAHK2`), CSV, JSON5, Kotlin, and Verilog. Their `SCE_*_*` enums are in `lexilla\lexers_x\SciXLexer.h`, not `lexilla\include\SciLexer.h`. `LexHTML.cxx` is under `lexilla\lexers\` in this repository; `SCLEX_HTML` and `SCLEX_XML` use that source.

## Fixtures and verification

`test\test_files\StyleLexers\styleLex<NAME>\` contains lexer fixtures. Any tokenization change (style, state-machine branch, keyword list, escape handling, fold rule, etc.) must extend or add a fixture. Open fixtures in Notepad3 for visual verification; automated lexer assertions are not in scope. Folder names follow the EDITLEXER source file; the historical AHK folder is `test\test_files\StyleLexers\styleLexAHKL\`.

Style-label resources are part of lexer work; general locale resource invariants and UI strings are documented in [Localization](localization.md).
