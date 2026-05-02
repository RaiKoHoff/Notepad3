# File Content Flags

Notepad3 recognizes a handful of markers **inside the file itself** that change how the file is opened or styled. Unlike command-line switches or INI settings, these are written into the document, travel with it, and apply automatically each time the file is loaded.

This page documents the user-actionable markers. Notepad3 also performs several automatic content detections (BOMs, line endings, UTF-8 validity); those are summarized at the end as context.

---

## Quick reference

| Marker | Where in file | Effect | Gating INI key |
|--------|---------------|--------|----------------|
| `.LOG` | First 4 bytes of line 1 | Append fresh date + time on initial open, caret placed below it | — (always on) |
| Emacs file variables | First N bytes or last N bytes (N = `FileVarScanBytes`) | Sets tab width, indent, wrap, edge columns, lexer | `NoFileVariables` |
| Vim modelines (`vim:`, `vi:`, `ex:`) | First / last N bytes | Sets tab width, shiftwidth, expandtab, wrap, textwidth, filetype | `NoFileVariables` |
| Encoding tag (`coding:` / `encoding:` / `charset:`) | First / last N bytes | Forces a specific encoding for load | `NoEncodingTags` (default: **on** — disables tags) |
| Shebang `#!…` | First line | Picks the lexer (Python, Perl, Bash, Ruby, Tcl, PHP, Node, Lua, Awk, R, PowerShell, Julia, Dart, Nim, …) | — (always on) |

The scan window `N` is configurable via `Settings2.FileVarScanBytes` (default `512`, range `256`–`2048`).

INI keys are documented in the [Configuration Reference](Configuration.md).
Note the asymmetry: **Emacs file variables are on by default; encoding tags are off by default.** Set `NoEncodingTags=0` in `[Settings2]` to enable parsing of `# coding: utf-8` and friends.

---

## `.LOG` auto-timestamp

If the very first bytes of the file are exactly `.LOG` and the next byte is a line break (or EOF), Notepad3 treats the file as a log book:

- On **initial open** the current date and time are appended to the document, on a new line at the end.
- The caret is placed at the start of the empty line **after** the timestamp, so you can immediately type a log entry.
- On **reload** (revert / autoreload) no new timestamp is appended; the caret restores to its pre-reload position.
- The MRU caret position is **not** persisted for `.LOG` files — every initial open lands on the freshly inserted timestamp, regardless of where you were last time.

This matches the classic Windows Notepad behaviour. There is no INI key to disable it; remove the `.LOG` line (or change it to anything else) to opt the file out.

The exact timestamp format is controlled by `Settings2.DateTimeFormat` (or `DateTimeLongFormat`). See the [Configuration Reference](Configuration.md).

**Example file:**

```
.LOG
2026-04-30 14:02   Started new project, baseline performance numbers below.
2026-05-01 09:18   Re-ran after profile patch — 12% faster.
```

Open it again and Notepad3 appends today's date on a new line for the next entry.

---

## Emacs file variables

A line containing `name: value` (or `name=value`, `name="value"`) anywhere in the head or tail scan window (default 512 bytes; configurable via `Settings2.FileVarScanBytes`) is parsed as an Emacs-style file variable. Notepad3 honors the following names (all case-insensitive):

| Variable | Type | Effect |
|----------|------|--------|
| `enable-local-variables` | int | Set to `0` to disable parsing of all other file variables in this file. |
| `tab-width` | int (1–256) | Tab stop width. |
| `c-basic-indent` | int (0–256) | Indent width (auto-indent / shift). |
| `indent-tabs-mode` | int (0 or 1) | `0` = insert spaces, `1` = insert tabs. |
| `c-tab-always-indent` | int (0 or 1) | Whether <kbd>Tab</kbd> always indents (vs. inserting a tab character at the cursor). |
| `truncate-lines` | int (0 or 1) | `0` = enable word-wrap, `1` = no wrap. |
| `fill-column` | string | One or more column numbers separated by spaces or commas — drawn as long-line edge markers. |
| `mode` | string | Lexer / language hint. See **Mode value matching** below. |

Comment characters and surrounding text are ignored — only the `name: value` token is recognized. The header is scanned first; if no variables are found there, Notepad3 falls back to scanning the tail of the file.

**Example (Python file with file variables in a trailing comment block):**

```python
# Local Variables:
# mode: python
# tab-width: 4
# indent-tabs-mode: 0
# c-basic-indent: 4
# fill-column: 80, 120
# End:
```

**Example (header form, single line):**

```c
/* -*- mode: c; tab-width: 8; indent-tabs-mode: 1 -*- */
```

To disable Emacs file-variable parsing globally, set `NoFileVariables=1` in `[Settings2]`. The same setting also gates Vim modeline parsing. See the [Configuration Reference](Configuration.md).

### Mode value matching

The `mode` value (or Vim's `ft=` value) is resolved in this order:

1. **Trailing suffix stripped** — common Emacs / Vim suffixes are removed: `-mode`, `-script`, `-major-mode`, `-minor-mode`. So `python-mode` → `python`, `markdown-mode` → `markdown`, `c#-mode` → `c#`.
2. **Extension list** — the (normalized) value is matched against each lexer's filename-extension list. So `mode: cpp`, `mode: c`, `mode: js`, `mode: go`, `mode: r` all resolve via this step.
3. **Display name prefix** — case-insensitive prefix match against the lexer's display name (e.g. `python` matches `"Python Script"`). Minimum length 2, so `mode: c#` resolves to the C# lexer.

If none of the steps match, the file's filename / extension fallback applies.

---

## Vim modelines

Vim-style modelines are parsed alongside Emacs file variables (gated by the same `NoFileVariables` setting) and are searched in the same head / tail window. Three marker prefixes are recognized: `vim:`, `vi:`, `ex:`. The marker must be preceded by whitespace or be at the start of a line. Both forms work:

- **Set form** (terminated by `:`): `vim: set ft=python ts=4 sw=4 noet :`
- **Colon form**: `vim:ft=python:ts=4:sw=4`

| Option (short / long) | Maps to | Notes |
|-----------------------|---------|-------|
| `ts` / `tabstop` | tab-width | int 1–256 |
| `sw` / `shiftwidth` | indent-width | int 0–256 |
| `et` / `expandtab` | indent-tabs-mode = spaces | flag (no value) |
| `noet` / `noexpandtab` | indent-tabs-mode = tabs | flag |
| `wrap` | word-wrap on | flag |
| `nowrap` | word-wrap off | flag |
| `tw` / `textwidth` | long-line edge marker | int |
| `ft` / `filetype` | mode (lexer hint) | uses the same matcher as Emacs `mode:` |

Other Vim options are silently ignored.

**Examples:**

```python
# vim: set ft=python ts=4 sw=4 et :
```

```c
// vim: ft=c ts=8 sw=8 noet :
```

```yaml
# vim:ft=yaml:ts=2:sw=2:et:
```

---

## Encoding tags

> **Default: off.** Encoding tag parsing is gated by `Settings2.NoEncodingTags`, which defaults to `1` (tags ignored). Set `NoEncodingTags=0` to enable. The default-off was chosen because any innocuous mention of `coding:` or `charset:` in code or comments would otherwise be honored.

If the file has **no BOM** and parsing is enabled, Notepad3 looks for an encoding declaration and uses it as the load encoding. The first match wins, in this order:

1. `coding: <name>`
2. `encoding: <name>`
3. `charset: <name>`

The same `name: value` / `name=value` / `name="value"` syntax as Emacs file variables; the same 512-byte head + tail scan window. The encoding name must be one Notepad3 knows (UTF-8, UTF-16, ISO-8859-*, Windows-*, the various code pages — anything visible in the encoding picker).

**Examples that work:**

```python
# -*- coding: utf-8 -*-
```

```html
<!-- charset="ISO-8859-15" -->
```

```ruby
# encoding: Windows-1252
```

A file with a BOM ignores the encoding tag — the BOM wins. To disable encoding-tag parsing globally, set `NoEncodingTags=1` in `[Settings2]`. See the [Configuration Reference](Configuration.md).

---

## Shebang lexer hint

If the first line begins with `#!`, Notepad3 extracts the interpreter token, normalizes it, and selects a matching lexer. Recognized forms:

- `#!/usr/bin/perl`
- `#!/usr/bin/env python3` — the `env` keyword is recognized; POSIX flags such as `-S` are skipped, and the next token is used.
- `#!/path/with spaces/python3.11` — any path prefix is stripped (only the basename matters).

**Normalization** applied to the basename before matching:

1. Strip leading path (everything before the last `/` or `\`).
2. Strip trailing version suffix — digits, dots, and dashes — so `python3.11` → `python`, `Rscript-4.3` → `Rscript`.
3. Compare case-insensitively against the table below; an exact (token-equal) match wins.

| Interpreter | Lexer |
|-------------|-------|
| `python` | Python |
| `perl` | Perl |
| `ruby` | Ruby |
| `php` | Web (HTML / PHP) |
| `node`, `nodejs`, `deno`, `bun` | JavaScript |
| `tclsh`, `wish`, `tcl` | Tcl |
| `sh`, `bash`, `zsh`, `ksh`, `dash`, `ash`, `fish`, `csh`, `tcsh` | Shell (Bash) |
| `lua`, `luajit` | Lua |
| `awk`, `gawk`, `mawk`, `nawk` | Awk |
| `Rscript`, `littler` | R |
| `pwsh`, `powershell` | PowerShell |
| `julia` | Julia |
| `dart` | Dart |
| `nim`, `nimrod` | Nim |

Bare `R` is intentionally **not** matched (too ambiguous as a single character — use `Rscript`, `littler`, or a `mode: r` file variable instead).

There is no INI key to disable shebang sniffing; rename or change the `#!` line to suppress it.

---

## How Notepad3 reads these markers

- **`.LOG`** is checked at offset 0 only; bytes 1–4 must be `.LOG`, byte 5 must be `\r`, `\n`, or EOF.
- **Emacs file variables**, **Vim modelines**, and **encoding tags** are searched in two passes: first the leading `Settings2.FileVarScanBytes` bytes, then — only if nothing was found in the head — the trailing window. Default 512, max 2048.
- **Shebang** is checked on line 1 only.

**Precedence for encoding** (most specific first):

1. Command-line switch (`/utf8`, `/ansi`, …) or `/e <name>`
2. Byte Order Mark (BOM) at the start of the file
3. Encoding tag (`coding:` / `encoding:` / `charset:`)
4. uchardet auto-detection
5. `Settings.DefaultEncoding`

**Precedence for lexer** (most specific first):

1. Command-line `/d`, `/h`, `/x`, `/s`
2. Filename / extension match
3. `mode:` file variable / `ft=` Vim modeline
4. Shebang sniff
5. Default lexer

---

## Related automatic detections (not file-content flags)

For completeness — these aren't markers you write, but Notepad3 reads file content to decide them:

- **BOM detection** — UTF-8 (`EF BB BF`), UTF-16 LE (`FF FE`), UTF-16 BE (`FE FF`), and UTF-32 BOMs are detected at file start and select the corresponding encoding. A BOM overrides any encoding tag.
- **Line-ending detection** — Notepad3 counts `\r\n`, `\r`, and `\n` occurrences and picks the most common as the document's EOL mode. If multiple kinds are mixed, you'll see the inconsistent-EOL warning (when enabled).
- **UTF-8 validity** — files whose non-ASCII bytes form valid UTF-8 sequences are loaded as UTF-8 (instead of the ANSI code page) when `LoadASCIIasUTF8` is on. Pure 7-bit ASCII files follow the same setting.

See [`readme/uchardet/EncodingDetection.md`](../uchardet/EncodingDetection.md) for the full encoding-detection pipeline and [Configuration Reference](Configuration.md) for the related `[Settings2]` keys (`NoFileVariables`, `NoEncodingTags`, `FileVarScanBytes`, `LoadASCIIasUTF8`, `LoadNFOasOEM`, `DateTimeFormat`, `DateTimeLongFormat`).
