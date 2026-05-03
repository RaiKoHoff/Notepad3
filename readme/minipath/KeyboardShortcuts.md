# MiniPath — Keyboard Shortcuts

[MiniPath](../../minipath/) is the lightweight file-browser companion to Notepad3, launched from inside Notepad3 with **`Ctrl+M`**. It is a fork of Florian Balmer's metapath, with cosmetic and 64-bit modernisation.

The bindings below are taken directly from `minipath/src/MiniPath.rc` (accelerator table). The in-app menus are the live source of truth — if a future build moves a binding, the menu shows the current key first.

> Looking for the **Notepad3** keyboard shortcuts? See [`../KeyboardShortcuts.md`](../KeyboardShortcuts.md).

---

## File / folder navigation

| Shortcut | Action |
|---|---|
| `Enter` | Open the selected file in the existing **target** Notepad3 window |
| `Shift+Enter` | Open the selected file in a **new** Notepad3 window |
| `Alt+Enter` | Show **Properties** of the selected object |
| `Backspace` | Move to the parent (upper) directory |
| `Ctrl+Backspace` | Move directly to the root directory |
| `Alt+←` | History: go back to the previous directory |
| `Alt+→` | History: go forward to the next directory |
| `Alt+↑` | Move selection to the **previous** file in the current folder |
| `Alt+↓` | Move selection to the **next** file in the current folder |
| `Ctrl+D` | Change to another directory… |
| `Alt+D` | Create a new directory |
| `Ctrl+G` | Goto path / URL… |
| `Alt+G` | Goto the target of the selected shortcut (`.lnk`) |
| `Tab` | Move focus to the next control |
| `Shift+Tab` | Move focus to the previous control |
| `Esc` | Exit (or minimise — configurable) |

---

## File operations

| Shortcut | Action |
|---|---|
| `Ctrl+L` | Launch the selected file with its associated application |
| `Alt+L` | "Open with…" — pick another application for the selected file |
| `Ctrl+F` | Launch the configured target application **without** any file argument |
| `Ctrl+R` | Run command dialog… |
| `Ctrl+N` | Create a new (empty) file in the current directory |
| `Ctrl+S` | Save the selected file to a new location ("Save copy as") |
| `Ctrl+K` | Create a shortcut (`.lnk`) to the selected object |
| `Ctrl+Q` | Open the selected file with the configured **quickviewer** |
| `F2` | Rename the selected object |
| `F5` | Refresh the directory listing |
| `F6` | Copy / move the selected object to another location… |
| `Del` | Delete the selected object (sends to the Recycle Bin) |
| `Shift+Del` | Delete the selected object **permanently** (skips the Recycle Bin) |
| `Ctrl+Shift+Del` | Delete the selected object **without confirmation** (and skip the Recycle Bin) |

---

## Sorting & filtering

| Shortcut | Action |
|---|---|
| `Ctrl+1` | Sort files by **Name** |
| `Ctrl+2` | Sort files by **Size** |
| `Ctrl+3` | Sort files by **Type** |
| `Ctrl+4` | Sort files by **Date** |
| `Ctrl+5` | Toggle **reverse** sort order |
| `Ctrl+M` | Set / change the file filter (e.g. `*.txt;*.md`)… |
| `Ctrl+*` (`Ctrl+Num*`) | Reset filter to `*.*` (show everything) |

---

## Target window (Notepad3) & launching

The "target" is the Notepad3 window that MiniPath is feeding files to.

| Shortcut | Action |
|---|---|
| `Ctrl+E` | Toggle **"activate target window"** — whether opening a file pulls Notepad3 to the foreground |
| `Ctrl+F12` | Setup MiniPath's target application (browse to the Notepad3 binary)… |

---

## Window position & appearance

| Shortcut | Action |
|---|---|
| `Ctrl+P` | Snap MiniPath next to the **target** Notepad3 window |
| `Ctrl+Shift+P` | Move window to MiniPath's default position |
| `Ctrl+T` | Toggle **"always on top"** |
| `Ctrl+U` | Toggle **transparency** mode |
| `Alt+S` | Toggle the status bar |
| `Alt+T` | Toggle the toolbar |
| `Alt+U` | Toggle the drive selection box |
| `Alt+N` | Open a new MiniPath window |

---

## Favorites

| Shortcut | Action |
|---|---|
| `F11` | Jump to the **Favorites** folder |
| `F9` | Edit (open in Explorer) the Favorites folder |

---

## Settings

| Shortcut | Action |
|---|---|
| `F7` | Save settings now |
| `Ctrl+F7` | Open the MiniPath INI file in the editor |
| `Ctrl+H` | Save settings now (alternate binding) |
| `F12` | Open MiniPath's program-settings dialog… |

---

## Help

| Shortcut | Action |
|---|---|
| `F1` | About MiniPath… |

---

## Cheatsheet — function keys

| Key | Plain | Ctrl | Alt |
|---|---|---|---|
| `F1` | About | — | — |
| `F2` | Rename | — | — |
| `F5` | Refresh | — | — |
| `F6` | Copy/move | — | — |
| `F7` | Save settings | Open INI | — |
| `F9` | Edit Favorites folder | — | — |
| `F11` | Jump to Favorites | — | — |
| `F12` | Program settings | Setup target app | — |

---

## See also

- [`../KeyboardShortcuts.md`](../KeyboardShortcuts.md) — Notepad3 keyboard shortcuts.
- [`../faq/FAQ.md`](../faq/FAQ.md) — frequently asked questions (incl. integration of MiniPath as the file-browser plugin).
- `minipath/Readme.txt` — MiniPath's bundled readme, including history and credits.
