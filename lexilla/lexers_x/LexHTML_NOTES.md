# `lexers_x/LexHTML.cxx` — design notes for the (paused) CSS-in-HTML extension

This file is **not currently compiled**. The standard upstream `lexers/LexHTML.cxx`
is what `Lexilla.lib` links against. This document captures everything we
learned while trying to make the extended (CSS-in-HTML) lexer work, so a future
session can pick up where this left off.

The extended lexer started as PR #5681 ("HTML CSS Styles") which patched the
upstream HTML lexer to also style embedded `<style>…</style>` blocks. The patch
worked superficially but had several latent bugs that surfaced under real use.
We solved most of them; one remained open when this work was paused.

---

## 1. Style-index collision (FIXED)

The PR placed `SCE_HCSS_DEFAULT … SCE_HCSS_OPERATOR` on Scintilla style
indices **32–39**, which Scintilla itself reserves for system styles
(`STYLE_DEFAULT`, `STYLE_LINENUMBER`, `STYLE_BRACELIGHT`, `STYLE_BRACEBAD`,
`STYLE_CONTROLCHAR`, `STYLE_INDENTGUIDE`, `STYLE_CALLTIP`,
`STYLE_FOLDDISPLAYTEXT`). This is exactly why upstream Lexilla's
`lexicalClassesHTML[]` deliberately documents 32–39 as `"predefined"`
empty placeholders.

The collision was visibly catastrophic for `SCE_HCSS_PROPERTY` (= 37 =
`STYLE_INDENTGUIDE`), where `Style_SetStyles()` interprets a numeric size in
the style string as the indent-guide width, not a font size — so CSS
properties never got a font set at all.

**Fix applied:** renumber the entire `SCE_HCSS_*` block contiguously to
**128–140**, safely above the predefined range AND above every other embedded
sub-language used by `SCLEX_HTML` (PHP ends at 127). The constants live in
`lexilla/lexers_x/SciXLexer.h` and the iface declaration in
`lexilla/lexers_x/SciX.iface`.

| Constant                | Index |
|-------------------------|-------|
| `SCE_HCSS_DEFAULT`      | 128   |
| `SCE_HCSS_COMMENT`      | 129   |
| `SCE_HCSS_SELECTOR`     | 130   |
| `SCE_HCSS_CLASS`        | 131   |
| `SCE_HCSS_ID`           | 132   |
| `SCE_HCSS_PROPERTY`     | 133   |
| `SCE_HCSS_VALUE`        | 134   |
| `SCE_HCSS_OPERATOR`     | 135   |
| `SCE_HCSS_STRING`       | 136   |
| `SCE_HCSS_PSEUDOCLASS`  | 137   |
| `SCE_HCSS_IMPORTANT`    | 138   |
| `SCE_HCSS_DIRECTIVE`    | 139   |
| `SCE_HCSS_NUMBER`       | 140   |

The original `SCE_H_…` placeholders 32–39 were restored as `"predefined"` rows
in `lexicalClassesHTML[]` (matches upstream).

---

## 2. CSS comment open/close pattern (FIXED)

The PR's CSS comment open/close used a manual `i++; ch = chNext; chNext =
SafeGetUnsignedCharAt(...)` pattern inside the `case` body, which is a
different idiom from the JS/PHP comment handling already in this lexer:

```cpp
// PR's pattern (unusual)
case SCE_HCSS_COMMENT:
    if (ch == '*' && chNext == '/') {
        i++; ch = chNext; chNext = SafeGetUnsignedCharAt(styler, i + 1);
        styler.ColourTo(i, SCE_HCSS_COMMENT);
        state = SCE_HCSS_DEFAULT;
    }
    break;

// JS/PHP idiom (already in this lexer)
case SCE_HJ_COMMENT:
    if (ch == '/' && chPrev == '*') {
        styler.ColourTo(i, StateToPrint);
        state = SCE_HJ_DEFAULT;
        ch = ' ';
    }
    break;
```

**Fix applied:** changed both open and close to the JS/PHP idiom.
- Open: `i++` only, no `ch = chNext` modification.
- Close: detect `chPrev == '*' && ch == '/'`.

Note: the `ch = ' '` after close (in JS) is *not* needed for CSS, because
CSS has no equivalent of the post-switch `if (state == SCE_HJ_DEFAULT)`
re-check that JS uses; the trailing `/` won't be misinterpreted, so we
omit it.

---

## 3. Missing `case SCE_HCSS_OPERATOR:` (FIXED)

`SCE_HCSS_OPERATOR` is only ever used as a one-shot colouring style for
`;`, `{`, `}`, `:`, etc. — `state` is *never* set to `OPERATOR` during
normal flow. But on incremental re-lex, Scintilla can hand the lexer
`initStyle = SCE_HCSS_OPERATOR` because the char before `startPos` was a
CSS operator. Without a `case SCE_HCSS_OPERATOR:` arm, the switch falls
through silently, `state` stays `OPERATOR`, no `ColourTo` calls fire, and
the entire region accumulates into a single bad segment.

**Fix applied:** add a recovery arm:

```cpp
case SCE_HCSS_OPERATOR:
    state = SCE_HCSS_DEFAULT;
    --i;
    continue;   // reprocess this char in DEFAULT state
```

---

## 4. `SCE_HCSS_COMMENT` / `SCE_HCSS_STRING` resume backtrack (FIXED)

Same shape as the upstream PHP heredoc/nowdoc problem: the closing `/` of
`*/` and the closing `"` of strings carry `SCE_HCSS_COMMENT` /
`SCE_HCSS_STRING` styles. If Scintilla re-lexes from a position past those,
`initStyle` says we're inside the comment/string and the lexer waits for a
terminator that already passed.

**Fix applied:** add both states to `StyleNeedsBacktrack()`. The existing
init code at the top of `Lex()` then walks backwards through chars carrying
those styles until it finds a stable char, and resumes from a known-good
line start.

---

## 5. **OPEN** — mid-CSS-token chunk-boundary recovery

This is where we paused. **Symptom**, reproducible on a fresh open of
`test/test_files/StyleLexers/styleLexHTML/MultiLang.html`: the CSS section
styles correctly up to some line (the exact line varies — observed at
line 47, 54, 62/63 in different sessions); from there to `</style>` every
character is rendered as `SCE_HCSS_DEFAULT` (= empty, inherits global
default; the user reads this as gray). The HTML below `</style>` is fine.
F5 (revert) progressively *moves the boundary later* — meaning each lex
pass styles slightly more correctly, but the bug persists on the unstyled
tail.

**Working theory.** Scintilla styles documents progressively (idle styling,
lazy paint, scroll-driven re-lex). Each new `Lex()` call derives `state`
from `initStyle = styler.StyleAt(startPos-1)`. Several CSS sub-states —
`PROPERTY`, `VALUE`, `NUMBER`, `SELECTOR`, `CLASS`, `ID`, `PSEUDOCLASS`,
`IMPORTANT`, `DIRECTIVE` — don't `ColourTo` until the token *ends*. If the
chunk boundary lands mid-token, those characters were never coloured, so
`styler.StyleAt(boundary-1)` returns 0 (`SCE_H_DEFAULT`). The next pass
wakes up thinking it's in HTML default, even though the line state still
correctly remembers `inScriptType == eNonHtmlScript` and
`clientScript == eScriptCSS`. The lexer treats CSS chars as `SCE_H_DEFAULT`
until `</style>` finally closes the script.

The progressive-F5 behaviour fits this: each F5 lets Scintilla style a bit
more, the boundary moves, and the gray region shrinks.

**Attempted fix that didn't resolve it.** At the top of `Lex()`, after the
existing `state == SCE_H_COMMENT` recovery, we added:

```cpp
if (inScriptType == eNonHtmlScript && state == SCE_H_DEFAULT &&
    clientScript == eScriptCSS) {
    state = SCE_HCSS_DEFAULT;
    scriptLanguage = eScriptCSS;
}
```

This compiles cleanly and *should* fix the reported scenario based on a
careful reading of the state-machine. But the user reported the bug
unchanged after rebuild — so either (a) the bug isn't actually triggered
by the path we modeled, or (b) the recovery needs to fire somewhere else
(e.g., on a later iteration, not just at `Lex()` init).

**What still needs investigation.**
- Get *real* per-character style output from the running lexer on this
  file (not from a Python simulator). Either by running a small C++ test
  that links against `Lexilla.lib` and dumps `styler.StyleAt()` for the
  region, or by adding temporary `OutputDebugString` calls inside `Lex()`
  to log `(startPos, length, initStyle, state, scriptLanguage, cssContext)`
  on every entry. This would confirm or refute the chunk-boundary theory.
- Check whether Scintilla's idle-styling chunking is what's calling
  `Lex()` here, or if it's a different path (paint-driven, scroll-driven,
  full-doc on initial set). The fix may need to live in a different place
  if the entry point is different.
- Consider adding `SCE_HCSS_PROPERTY` (and the other token states) to
  `StyleNeedsBacktrack()`. That would force Scintilla to walk backwards
  through unstyled chars to find a stable position — even if those chars
  were never coloured. (Not obvious whether `StyleNeedsBacktrack` triggers
  on the *style* of the resume position or on the lexer's reconstructed
  `state`; need to read Scintilla source.)
- Compare side-by-side with `lexilla/lexers/LexCSS.cxx` (upstream
  standalone CSS lexer) — it uses `StyleContext` and a `lastStateC`
  backtrack inside the comment-close to *actively look up* the correct
  resume state. Adopting that pattern more thoroughly may sidestep the
  problem entirely.

**State at pause.** All of fixes 1–4 are in
`lexilla/lexers_x/LexHTML.cxx`. The fix for 5 (the resume-recovery in
`Lex()` init) is also there but doesn't fully work. The Notepad3
`StyleLexers/styleLexHTML.c` and `Styles.c` CSS additions, plus the
language string IDs, plus the dark-mode scheme entries, plus the
`Lexilla.vcxproj` switch from `lexers/` to `lexers_x/` — all are reverted
so the project once again compiles upstream's pristine HTML lexer (no CSS
in `<style>`). The extended file in `lexers_x/` is preserved verbatim for
when this work resumes.

---

## 6. Reproduction setup for next session

1. Open `test/test_files/StyleLexers/styleLexHTML/MultiLang.html` in
   Notepad3. Note where the CSS section "goes gray".
2. Press F5 (revert) and watch the gray region shrink.
3. To switch back to the extended lexer: in
   `lexilla/Lexilla.vcxproj` swap the ClCompile path from
   `lexers\LexHTML.cxx` to `lexers_x\LexHTML.cxx`, swap the same in
   `Lexilla.vcxproj.filters` (and put the filter under `lexers_x`),
   re-apply the Notepad3 core changes from PR #5681 (use `git diff
   bc7f39a4c~1 bc7f39a4c -- src/StyleLexers/styleLexHTML.c src/Styles.c
   language/ res/StdDarkModeScheme.ini` to see the original deltas), and
   build.

---

## 7. Useful references

- Upstream Lexilla HTML lexer:
  `O:/DEV/GitHubExt/lexilla/lexers/LexHTML.cxx` (read-only reference)
- Upstream Lexilla CSS lexer (different paradigm — uses StyleContext):
  `lexilla/lexers/LexCSS.cxx`. Worth porting its backtrack/`lastStateC`
  pattern into `lexers_x/LexHTML.cxx`.
- `Scintilla.h:229–237` — definitive reference for Scintilla's
  reserved style indices 32–39.
- `lexilla/lexers/LexHTML.cxx` JS comment close (lines around 2285) and
  PHP comment close (lines around 2785) — the idiom we matched in the
  CSS comment-close fix.
- The PR that started this: `bc7f39a4c feat: HTML CSS Styles`. Use
  `git show bc7f39a4c` for the full original delta.
