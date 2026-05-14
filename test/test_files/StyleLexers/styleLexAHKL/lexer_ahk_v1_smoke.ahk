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
#IfWinActive ahk_class Notepad          ; conditional-directive context (WinTitle, NOT expression)
#If WinActive("Notepad") && A_Hour > 9  ; #If expression context (plan §6 D2)

; --- escape character + special chars in strings -------------------------
str := "hello`tworld`n""quoted`"""      ; backtick escapes, doubled quote
path := "C:\Users\foo\bar.txt"

; --- numbers -------------------------------------------------------------
ints   := 42
hex    := 0xFF
flt    := 3.14
neg    := -7
sci1   := 1e5                            ; scientific notation (plan §6 L4)
sci2   := 1.5e-3
sci3   := 0.5E+10

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
  Send {Tab}{Enter 3}                    ; send-key braces -> WORD_KB, no fold (plan §6 B3)
  Send {{}MyVar{}}                       ; literal `{` / `}` send (unbalanced count without B3)
  SendInput {F5}{F10}                    ; SendInput recognised
  ControlSend, , {Enter}, ahk_class Notepad
  Run notepad.exe
  return

; B3 false-positive guard: Send used as a variable/identifier, NOT a command call.
; The `{` after `if (Send)` must stay SYNOPERATOR so the code block folds correctly.
if (Send) {
  MsgBox, var-mode Send
}
result := MyFunc(Send, otherVar)         ; Send as function arg, not command

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

; --- continuation sections (plan §6 D1) ---------------------------------
; literal-string continuation (default — body styles as STRING)
msg :=
(
Line one of the message.
Line two with `tcontrol chars.
Last line.
)

; expression-mode continuation via Join option — body styles as expression
total := 0 +
(Join
    1 + 2
  + 3 * 4
  + StrLen("hello")
)

; --- doc-comment style ;@ (AHK_L convention, plan §6 L3) ----------------
;@function MakePoint
;@param x - the x coordinate
;@param y - the y coordinate
;@returns Point object
MakePoint(x, y) {
  return {x: x, y: y}
}

; --- reference operator &var (plan §6 L5) -------------------------------
GetSize(&outW, &outH) {                  ; ByRef parameters
  outW := 800
  outH := 600
}
GetSize(&w, &h)

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
