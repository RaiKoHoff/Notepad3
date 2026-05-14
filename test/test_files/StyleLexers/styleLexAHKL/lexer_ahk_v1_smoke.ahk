; ============================================================================
;  Notepad3 AutoHotkey v1 lexer smoke test
;  Open this file in Notepad3 to visually verify every SCE_AHK_* style class.
;  No script logic is meant to execute; this is a lexer fixture only.
; ============================================================================

; --- line comment + block comment ----------------------------------------
/*
   block comment, /* and */ must start a line (whitespace allowed)
*/

; --- directives (#-words) ------------------------------------------------
#NoEnv
#SingleInstance Force
#Include %A_ScriptDir%\lib.ahk          ; var deref inside a directive arg
#IfWinActive ahk_class Notepad          ; conditional-directive context

; --- escape character + special chars in strings -------------------------
str := "hello`tworld`n""quoted`"""      ; backtick escapes, doubled quote
path := "C:\Users\foo\bar.txt"

; --- numbers -------------------------------------------------------------
ints   := 42
hex    := 0xFF
flt    := 3.14
neg    := -7
; lexer currently lumps `1e5` as identifier; see plan §3 (scientific notation)
;sci   := 1e5

; --- expression assignment + multi-char operators ------------------------
result := 3 + 4 * (2 ** 8)
ok     := (x >= 10) && (y != "z") || !done
tern   := cond ? "yes" : "no"           ; mid-line `:` -> SYNOPERATOR (fix §1.2)

; --- object/map literals (v1.1+) ----------------------------------------
obj := {key: "value", n: 42}            ; `:` after key -> SYNOPERATOR
arr := [1, 2, 3, "four"]

; --- variable dereferencing ---------------------------------------------
name := "World"
MsgBox, Hello, %name%!                  ; legacy command + %var% deref

; --- hotstring with options ---------------------------------------------
:*?B0:btw::by the way                    ; option run `:*?B0:` (plan §2.6 split)
::omg::Oh my gosh                        ; bare hotstring

; --- hotkeys ------------------------------------------------------------
^!a::                                    ; Ctrl+Alt+a
  Send {Tab}{Enter 3}                    ; send-key braces (plan §2.7 / §4)
  Run notepad.exe
  return

~LButton & RButton::                     ; custom combo with modifiers
  Tooltip, Combo!
  return

; --- labels + Loop block -----------------------------------------------
MyLabel:
  Loop, 5 {
    FileAppend, %A_Index%`n, out.txt
  }
  return

; --- try / catch / throw -----------------------------------------------
try {
  throw "boom"
} catch e {
  MsgBox, % "caught: " e
}

; --- class / extends / method / static (v1.1+) -------------------------
class Animal {
  static count := 0
  __New(name) {
    this.name := name
    Animal.count++
  }
  Speak() {
    MsgBox, % this.name " speaks"
  }
}

class Dog extends Animal {
  Speak() {
    MsgBox, % this.name " barks"
  }
}

; --- GUI command --------------------------------------------------------
Gui, Add, Text,, Demo
Gui, Show
return

; --- built-in variables -------------------------------------------------
MsgBox, % A_ScriptDir " | " A_Now " | " A_UserName

ExitApp
