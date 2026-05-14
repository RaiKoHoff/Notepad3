// encoding: UTF-8
// Scintilla source code edit control
/** @file LexAHK2.cxx
 ** Lexer for AutoHotkey v2, simplified version
 ** Written by Philippe Lhoste (PhiLho)
 **/
 // Copyright 1998-2006 by Neil Hodgson <neilh@scintilla.org>
 // The License.txt file describes the conditions under which this software may be distributed.
 /* notepad2-mod custom code for the AutoHotkey lexer */


#include <string>
#include <string.h>     // _stricmp, _strnicmp (MSVC CRT, case-insensitive ASCII)
#include <assert.h>
#include <map>
//
#include "ILexer.h"
#include "Scintilla.h"
#include "StringCopy.h"
#include "PropSetSimple.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "LexerModule.h"
#include "DefaultLexer.h"
#include "OptionSet.h"
#include "WordList.h"
//
#include "CharSetX.h"
#include "SciXLexer.h"


using namespace Lexilla;
using namespace Scintilla;


// Options used for LexerRust
struct OptionsAHK
{
    bool fold;
    bool foldComment;
    bool foldCompact;
    OptionsAHK() 
        : fold(false)
        , foldComment(true)
        , foldCompact(true)
    { }
};

static const char* const ahk2WordLists[] =
{
    "Flow of Control",
    "Commands",
    "Functions",
    "Directives",
    "Keys & Buttons",
    "Variables",
    "Special Parameters (keywords)",
    "User Defined",
    nullptr
};



struct OptionSetAHK : public OptionSet<OptionsAHK>
{
    OptionSetAHK()
    {
        DefineProperty("fold", &OptionsAHK::fold);
        DefineProperty("fold.comment", &OptionsAHK::foldComment);
        DefineProperty("fold.compact", &OptionsAHK::foldCompact);

        DefineWordListSets(ahk2WordLists);
    }
};

class LexerAHK2 : public DefaultLexer
{

    OptionsAHK options;
    OptionSetAHK osAHK;

    WordList controlFlow;
    WordList commands;
    WordList functions;
    WordList directives;
    WordList keysButtons;
    WordList variables;
    WordList specialParams;
    WordList userDefined;

    CharacterSet WordChar;
    CharacterSet ExpOperator;

public:
    LexerAHK2() : DefaultLexer("AHK2", SCLEX_AHK2, nullptr, 0),
        //valLabel(CharacterSet::setAlphaNum, "@#$_~!^&*()+[]\';./\\<>?|{}-=\""),
        WordChar(CharacterSet::setAlphaNum, "@#$_[]?"),
        ExpOperator(CharacterSet::setNone, "+-*/!~&|^=<>.()")
    {
    }

    virtual ~LexerAHK2() = default;

    void SCI_METHOD Release() override
    {
        delete this;
    }
    int SCI_METHOD Version() const override
    {
        return lvRelease5;
    }
    const char* SCI_METHOD PropertyNames() override
    {
        return osAHK.PropertyNames();
    }
    int SCI_METHOD PropertyType(const char* name) override
    {
        return osAHK.PropertyType(name);
    }
    const char* SCI_METHOD PropertyGet(const char* name) override
    {
        return osAHK.PropertyGet(name);
    }
    const char* SCI_METHOD DescribeProperty(const char* name) override
    {
        return osAHK.DescribeProperty(name);
    }
    const char* SCI_METHOD DescribeWordListSets() override
    {
        return osAHK.DescribeWordListSets();
    }
    void * SCI_METHOD PrivateCall(int, void *) override
    {
        return nullptr;
    }

    // --------------------------------------------------------------------------
    Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override;
    Sci_Position SCI_METHOD WordListSet(int n, const char *wl) override;
    void SCI_METHOD Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess) override;
    void SCI_METHOD Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess) override;

    static ILexer5 *LexerFactoryAHK2()
    {
        return new LexerAHK2();
    }

private:
    void HighlightKeyword(char currentWord[], StyleContext& sc);

};

Sci_Position SCI_METHOD LexerAHK2::PropertySet(const char* key, const char* val)
{
    if (osAHK.PropertySet(&options, key, val))
    {
        return 0;
    }
    return -1;
}


Sci_Position SCI_METHOD LexerAHK2::WordListSet(int n, const char *wl)
{
    WordList *wordListN = nullptr;
    switch (n)
    {

    case 0:
        wordListN = &controlFlow;
        break;

    case 1:
        wordListN = &commands;
        break;

    case 2:
        wordListN = &functions;
        break;

    case 3:
        wordListN = &directives;
        break;

    case 4:
        wordListN = &keysButtons;
        break;

    case 5:
        wordListN = &variables;
        break;

    case 6:
        wordListN = &specialParams;
        break;

    case 7:
        wordListN = &userDefined;
        break;

    }

    int firstModification = -1;
    if (wordListN) {
        // AHK is fully case-insensitive for keywords/commands/directives/etc.
        // Load each wordlist lowercased; callers use sc.GetCurrentLowered()
        // (Lexilla idiom — see LexVB, LexSQL, LexAU3 for the same pattern).
        if (wordListN->Set(wl, true)) {
            firstModification = 0;
        }
    }
    return firstModification;
}


void LexerAHK2::HighlightKeyword(char currentWord[], StyleContext& sc) {

    // Wordlists are stored lowercase (see WordListSet); callers MUST pass an
    // already-lowercased buffer (use sc.GetCurrentLowered() at the call site).
    if (controlFlow.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_CF);
    }
    // v2 has NO command-mode syntax — every former v1 command (MsgBox, Send, …)
    // is now a function call. The commands wordlist arm still exists for index
    // parity with the v1 lexer (C-δ will empty the v2 wordlist itself), but we
    // intentionally skip the InList check so nothing is ever styled WORD_CMD.
    else if (functions.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_FN);
    }
    else if (currentWord[0] == '#' && directives.InList(currentWord + 1)) {
        sc.ChangeState(SCE_AHK_WORD_DIR);
    }
    else if (keysButtons.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_KB);
    }
    else if (variables.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_VAR);
    }
    else if (specialParams.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_SP);
    }
    else if (userDefined.InList(currentWord)) {
        sc.ChangeState(SCE_AHK_WORD_UD);
    }
    else {
        sc.ChangeState(SCE_AHK_DEFAULT);
    }
}



void SCI_METHOD LexerAHK2::Lex(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess)
{
    PropSetSimple props;
    Accessor styler(pAccess, &props);
    StyleContext sc(startPos, lengthDoc, initStyle, styler);

    // Do not leak onto next line
    if (initStyle != SCE_AHK_COMMENTBLOCK &&
        initStyle != SCE_AHK_STRING) {
        initStyle = SCE_AHK_DEFAULT;
    }
    int currentState = initStyle;
    int nextState = -1;
    char currentWord[256];

    /* The AutoHotkey syntax is heavily context-dependent.
    For example, for each command, the lexer knows if parameter #n
    is a string, a variable, a number, an expression, etc.
    I won't go this far, but I will try to handle most regular cases.
    */
    // True if in a continuation section
    bool bContinuationSection = (initStyle == SCE_AHK_STRING);
    // True if the active continuation section is expression-mode (Join opt etc.)
    bool bExprContinuation = false;
    // Indicate if the lexer has seen only spaces since the start of the line
    bool bOnlySpaces = (!bContinuationSection);
    // Indicate if since the start of the line, lexer met only legal label chars
    bool bIsLabel = false;
    // Distinguish hotkeys from hotstring
    bool bIsHotkey = false;
    bool bIsHotstring = false;
    // In an expression
    bool bInExpression = false;
    // A quoted string in an expression (share state with continuation section string)
    bool bInExprString = false;
    // To accept A-F chars in a number
    bool bInHexNumber = false;
    // To accept digits after e/E in scientific notation
    bool bInExponent = false;
    // (v2: no Send-args / line-start-ident tracking — v2 has no command-mode,
    //  so Send is a normal function call and key sequences live inside quoted strings.)


    for (; sc.More(); sc.Forward()) {
        if (nextState >= 0) {
            // I need to reset a state before checking new char
            sc.SetState(nextState);
            nextState = -1;
        }
        if (sc.state == SCE_AHK_SYNOPERATOR) {
            // Only one char (if two detected, we move Forward() anyway)
            sc.SetState(SCE_AHK_DEFAULT);
        }
        if (sc.atLineEnd && (bIsHotkey || bIsHotstring)) {
            // I make the hotkeys and hotstrings more visible
            // by changing the line end to LABEL style (if style uses eolfilled)
            bIsHotkey = bIsHotstring = false;
            sc.SetState(SCE_AHK_LABEL);
        }
        if (sc.atLineStart) {
            if (sc.state != SCE_AHK_COMMENTBLOCK &&
                !bContinuationSection) {
                // Prevent some styles from leaking back to previous line
                sc.SetState(SCE_AHK_DEFAULT);
            }
            bOnlySpaces = true;
            bIsLabel = false;
            bInExpression = false;	// I don't manage multiline expressions yet!
            bInHexNumber = false;
            bInExponent = false;
        }

        // Manage cases occuring in (almost) all states (not in comments)
        if (sc.state != SCE_AHK_COMMENTLINE &&
            sc.state != SCE_AHK_COMMENTBLOCK &&
            sc.state != SCE_AHK_COMMENTDOC &&
            !IsASpace(sc.ch)) {
            if (sc.ch == '`') {
                // Backtick, escape sequence
                currentState = sc.state;
                sc.SetState(SCE_AHK_ESCAPE);
                sc.Forward();
                nextState = currentState;
                continue;
            }
            // v2: the v1 `%var%` deref outside strings and `% ` expression-start
            // marker are gone. Variables are bare identifiers in expressions; `%`
            // alive only as the dynamic-deref operator inside strings (handled by
            // C-γ-4 expression-string interpolation). Anything `%` does outside a
            // quoted string in v2 is either a modulo operator or an error.
            if (sc.state != SCE_AHK_STRING && !bInExpression) {
                // Management of labels, hotkeys, hotstrings and remapping

                // Check if the starting string is a label candidate
                if (bOnlySpaces &&
                    sc.ch != ',' && sc.ch != ';' && sc.ch != ':' &&
                    sc.ch != '%' && sc.ch != '`') {
                    // A label cannot start with one of the above chars
                    bIsLabel = true;
                }

                // The current state can be IDENTIFIER or DEFAULT,
                // depending if the label starts with a word char or not
                if (bIsLabel && sc.ch == ':' &&
                    (IsASpace(sc.chNext) || sc.atLineEnd)) {
                    // ?l/a|b\e^l!:
                    // Only ; comment should be allowed after
                    sc.ChangeState(SCE_AHK_LABEL);
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    nextState = SCE_AHK_DEFAULT;
                    continue;
                }
                else if (sc.Match(':', ':')) {
                    if (bOnlySpaces) {
                        // Hotstring ::aa::Foo
                        bIsHotstring = true;
                        sc.SetState(SCE_AHK_SYNOPERATOR);
                        sc.Forward();
                        nextState = SCE_AHK_LABEL;
                        continue;
                    }
                    // Hotkey F2:: or remapping a::b
                    bIsHotkey = true;
                    // Check if it is a known key
                    sc.GetCurrentLowered(currentWord, sizeof(currentWord));
                    if (keysButtons.InList(currentWord)) {
                        sc.ChangeState(SCE_AHK_WORD_KB);
                    }
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    sc.Forward();
                    if (bIsHotstring) {
                        nextState = SCE_AHK_STRING;
                    }
                    continue;
                }
            }
        }
        // Check if the current string is still a label candidate
        // Labels are much more permissive than regular identifiers...
        if (bIsLabel &&
            (sc.ch == ',' || sc.ch == '%' || sc.ch == '`' || IsASpace(sc.ch))) {
            // Illegal character in a label
            bIsLabel = false;
        }

        // Determine if the current state should terminate.
        if (sc.state == SCE_AHK_COMMENTLINE || sc.state == SCE_AHK_COMMENTDOC) {
            if (sc.atLineEnd) {
                sc.SetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_COMMENTBLOCK) {
            if (bOnlySpaces && sc.Match('*', '/')) {
                // End of comment at start of line (skipping white space)
                sc.Forward();
                sc.ForwardSetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_EXPOPERATOR) {
            if (!ExpOperator.Contains(sc.ch)) {
                sc.SetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_STRING) {
            if (bContinuationSection && !bExprContinuation) {
                if (bOnlySpaces && sc.ch == ')') {
                    // End of literal-string continuation section
                    bContinuationSection = false;
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                }
            }
            else if (bInExprString) {
                if (sc.ch == '\"') {
                    if (sc.chNext == '\"') {
                        // In expression string, double quotes are doubled to escape them
                        sc.Forward();	// Skip it
                    }
                    else {
                        bInExprString = false;
                        sc.ForwardSetState(SCE_AHK_DEFAULT);
                    }
                }
                else if (sc.ch == '%' && WordChar.Contains(sc.chNext)) {
                    // v2 string interpolation: "Hello %name%" -- %expr% inside a
                    // quoted string is a dynamic deref. Save STRING state, hand off
                    // to VARREF; the existing VARREF handler returns to currentState
                    // (STRING) on the closing `%`.
                    currentState = sc.state;
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    nextState = SCE_AHK_VARREF;
                    continue;
                }
                else if (sc.atLineEnd) {
                    sc.ChangeState(SCE_AHK_ERROR);
                }
            }
            else {
                if (sc.ch == ';' && IsASpace(sc.chPrev)) {
                    // Line comments after code must be preceded by a space
                    sc.SetState(SCE_AHK_COMMENTLINE);
                }
            }
        }
        else if (sc.state == SCE_AHK_NUMBER) {
            if (bInHexNumber) {
                if (!IsADigit(sc.ch, 16)) {
                    bInHexNumber = false;
                    sc.SetState(SCE_AHK_DEFAULT);
                }
            }
            else if (bInExponent) {
                if (!IsADigit(sc.ch)) {
                    bInExponent = false;
                    sc.SetState(SCE_AHK_DEFAULT);
                }
            }
            else if ((sc.ch == 'e' || sc.ch == 'E') &&
                     (IsADigit(sc.chNext) || sc.chNext == '+' || sc.chNext == '-')) {
                // Scientific notation: enter exponent sub-state, optionally consume sign
                bInExponent = true;
                if (sc.chNext == '+' || sc.chNext == '-') {
                    sc.Forward();
                }
            }
            else if (!(IsADigit(sc.ch) || sc.ch == '.')) {
                sc.SetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_IDENTIFIER) {
            if (!WordChar.Contains(sc.ch)) {
                sc.GetCurrentLowered(currentWord, sizeof(currentWord));
                HighlightKeyword(currentWord, sc);
                // AHK_L user-function call: unknown identifier immediately followed by `(`
                if (sc.state == SCE_AHK_DEFAULT && sc.ch == '(') {
                    sc.ChangeState(SCE_AHK_WORD_UD);
                }
                // (v2: no Send-args detection — Send is a regular function call
                //  with quoted args; key sequences live INSIDE the quoted strings.)

                // AHK keywords are case-insensitive; flow-of-control `if` and the
                // `#If <expr>` directive both put the rest of the line in expression
                // mode. `#IfWin*` variants take a WinTitle, NOT an expression — they
                // are excluded by the exact match on "#If".
                if (_stricmp(currentWord, "if") == 0 ||
                    _stricmp(currentWord, "#If") == 0) {
                    bInExpression = true;
                }
                sc.SetState(SCE_AHK_DEFAULT);
            }
        }
        else if (sc.state == SCE_AHK_VARREF) {
            if (sc.ch == '%') {
                // End of variable reference
                sc.GetCurrentLowered(currentWord, sizeof(currentWord));
                if (variables.InList(currentWord)) {
                    sc.ChangeState(SCE_AHK_VARREFKW);
                }
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = currentState;
                continue;
            }
            else if (!WordChar.Contains(sc.ch)) {
                // Oops! Probably no terminating %
                sc.ChangeState(SCE_AHK_ERROR);
            }
        }
        else if (sc.state == SCE_AHK_LABEL) {
            // Hotstring -- modifier or trigger string :*:aa::Foo or ::aa::Foo
            if (sc.ch == ':') {
                sc.SetState(SCE_AHK_SYNOPERATOR);
                if (sc.chNext == ':') {
                    sc.Forward();
                }
                nextState = SCE_AHK_LABEL;
                continue;
            }
        }
        else if (sc.state == SCE_AHK_HOTSTRINGOPT) {
            // End of hotstring option run :*?B0: -> abbr
            if (sc.ch == ':') {
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = SCE_AHK_LABEL;
                continue;
            }
        }

        // Determine if a new state should be entered
        if (sc.state == SCE_AHK_DEFAULT) {
            if (sc.ch == ';' &&
                (bOnlySpaces || IsASpace(sc.chPrev))) {
                // Line comments are alone on the line or are preceded by a space.
                // AHK_L doc-comment convention: ";@" prefix (e.g. ;@param, ;@returns)
                if (sc.chNext == '@') {
                    sc.SetState(SCE_AHK_COMMENTDOC);
                } else {
                    sc.SetState(SCE_AHK_COMMENTLINE);
                }
            }
            else if (bOnlySpaces && sc.Match('/', '*')) {
                // Comment at start of line (skipping white space)
                sc.SetState(SCE_AHK_COMMENTBLOCK);
                sc.Forward();
            }
            else if (sc.ch == '{' || sc.ch == '}') {
                // v2: braces are always code-block / object-literal braces (Send-key
                // braces in v2 live INSIDE quoted strings, so `{`/`}` mid-line are
                // structural).
                sc.SetState(SCE_AHK_SYNOPERATOR);
            }
            else if (bExprContinuation && bContinuationSection && bOnlySpaces && sc.ch == ')') {
                // End of expression-mode continuation section (parallel to STRING-mode
                // end-detection at the SCE_AHK_STRING handler above).
                bContinuationSection = false;
                bExprContinuation = false;
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = SCE_AHK_DEFAULT;
            }
            else if (bOnlySpaces && sc.ch == '(') {
                // Continuation section. AHK_L: with explicit `Join` option the body is
                // an expression continuation; otherwise it's a multi-line string literal.
                bContinuationSection = true;
                bExprContinuation = false;
                {
                    // Scan rest of the `(` line (case-insensitive) for whole-word "Join".
                    Sci_PositionU const docEnd = startPos + lengthDoc;
                    for (Sci_PositionU p = sc.currentPos + 1; p < docEnd; ++p) {
                        const char c0 = styler.SafeGetCharAt(p);
                        if (c0 == '\n' || c0 == '\r') break;
                        if (((c0 & 0xDF) == 'J') &&
                            ((styler.SafeGetCharAt(p+1) & 0xDF) == 'O') &&
                            ((styler.SafeGetCharAt(p+2) & 0xDF) == 'I') &&
                            ((styler.SafeGetCharAt(p+3) & 0xDF) == 'N')) {
                            // Require whole-word boundary on both sides
                            const char cPrev = (p == 0) ? ' ' : styler.SafeGetCharAt(p - 1);
                            const char cNext = styler.SafeGetCharAt(p + 4);
                            if (!IsAlphaNumeric(cPrev) && !IsAlphaNumeric(cNext)) {
                                bExprContinuation = true;
                                break;
                            }
                        }
                    }
                }
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = bExprContinuation ? SCE_AHK_DEFAULT : SCE_AHK_STRING;
            }
            else if (sc.Match('=', '>')) {
                // v2 fat-arrow lambda:  MyFunc(x) => x * 2
                // Body after `=>` is an expression.
                bInExpression = true;
                sc.SetState(SCE_AHK_SYNOPERATOR);
                sc.Forward();
                nextState = SCE_AHK_DEFAULT;
            }
            else if (sc.Match(':', '=') ||
                sc.Match('+', '=') ||
                sc.Match('-', '=') ||
                sc.Match('/', '=') ||
                sc.Match('*', '=') ||
                sc.Match('.', '=') ||
                sc.Match('|', '=') ||
                sc.Match('&', '=') ||
                sc.Match('^', '=')) {
                // Expression assignment (v2 adds `.=`, `|=`, `&=`, `^=` beyond v1)
                bInExpression = true;
                sc.SetState(SCE_AHK_SYNOPERATOR);
                sc.Forward();
                nextState = SCE_AHK_DEFAULT;
            }
            else if (sc.ch == '.' && WordChar.Contains(sc.chPrev) && WordChar.Contains(sc.chNext)) {
                // AHK_L member access: ident.ident (distinct from concat/expr operator)
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = SCE_AHK_DEFAULT;
            }
            else if (ExpOperator.Contains(sc.ch)) {
                sc.SetState(SCE_AHK_EXPOPERATOR);
            }
            else if (sc.ch == '\"') {
                bInExprString = true;
                sc.SetState(SCE_AHK_STRING);
            }
            else if (sc.ch == '0' && (sc.chNext == 'x' || sc.chNext == 'X')) {
                // Hexa, skip forward as we don't accept any other alpha char (beside A-F) inside
                bInHexNumber = true;
                sc.SetState(SCE_AHK_NUMBER);
                sc.Forward(2);
            }
            else if (IsADigit(sc.ch) || (sc.ch == '.' && IsADigit(sc.chNext))) {
                sc.SetState(SCE_AHK_NUMBER);
            }
            else if (WordChar.Contains(sc.ch)) {
                sc.SetState(SCE_AHK_IDENTIFIER);
            }
            else if (sc.ch == ',') {
                sc.SetState(SCE_AHK_SYNOPERATOR);
                nextState = SCE_AHK_DEFAULT;
            }
            else if (sc.ch == ':') {
                if (bOnlySpaces) {
                    // Start of hotstring :opts:abbr::Stuff or ::abbr::Stuff
                    bIsHotstring = true;
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    if (sc.chNext == ':') {
                        // bare ::abbr::Stuff (no option run)
                        sc.Forward();
                        nextState = SCE_AHK_LABEL;
                    }
                    else {
                        // :opts:abbr::Stuff -- color the option chars distinctly
                        nextState = SCE_AHK_HOTSTRINGOPT;
                    }
                }
                else {
                    // Mid-line single colon: ternary `?:`, object/map literal key:value
                    sc.SetState(SCE_AHK_SYNOPERATOR);
                    nextState = SCE_AHK_DEFAULT;
                }
            }
        }
        if (!IsASpace(sc.ch)) {
            bOnlySpaces = false;
        }
    }
    // End of file: complete any pending changeState
    if (sc.state == SCE_AHK_IDENTIFIER) {
        sc.GetCurrentLowered(currentWord, sizeof(currentWord));
        HighlightKeyword(currentWord, sc);
    }
    else if (sc.state == SCE_AHK_STRING && bInExprString) {
        sc.ChangeState(SCE_AHK_ERROR);
    }
    else if (sc.state == SCE_AHK_VARREF) {
        sc.ChangeState(SCE_AHK_ERROR);
    }
    sc.Complete();

}


void SCI_METHOD LexerAHK2::Fold(Sci_PositionU startPos, Sci_Position lengthDoc, int initStyle, IDocument *pAccess)
{
    if (!options.fold) { return; }

    PropSetSimple props;
    Accessor styler(pAccess, &props);

    bool const foldComment = options.foldComment; //props.GetInt("fold.comment") != 0;
    bool const foldCompact = options.foldCompact; //props.GetInt("fold.compact", 1) != 0;

    Sci_PositionU endPos = startPos + lengthDoc;

    bool bOnlySpaces = true;
    int lineCurrent = styler.GetLine(startPos);
    int levelCurrent = SC_FOLDLEVELBASE;
    if (lineCurrent > 0) {
        levelCurrent = styler.LevelAt(lineCurrent - 1) & SC_FOLDLEVELNUMBERMASK;
    }
    int levelNext = levelCurrent;
    char chNext = styler[startPos];
    int styleNext = styler.StyleAt(startPos);
    int style = initStyle;
    for (Sci_PositionU i = startPos; i < endPos; i++) {
        char ch = chNext;
        chNext = styler.SafeGetCharAt(i + 1);
        int stylePrev = style;
        style = styleNext;
        styleNext = styler.StyleAt(i + 1);
        bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
        if (foldComment && style == SCE_AHK_COMMENTBLOCK) {
            if (stylePrev != SCE_AHK_COMMENTBLOCK) {
                levelNext++;
            }
            else if ((styleNext != SCE_AHK_COMMENTBLOCK) && !atEOL) {
                // Comments don't end at end of line and the next character may be unstyled.
                levelNext--;
            }
        }
        if (style == SCE_AHK_SYNOPERATOR) {
            if (ch == '(' || ch == '{') {
                levelNext++;
            }
            else if (ch == ')' || ch == '}') {
                levelNext--;
            }
        }
        if (atEOL) {
            int level = levelCurrent;
            if (bOnlySpaces && foldCompact) {
                // Empty line
                level |= SC_FOLDLEVELWHITEFLAG;
            }
            if (!bOnlySpaces && levelNext > levelCurrent) {
                level |= SC_FOLDLEVELHEADERFLAG;
            }
            if (level != styler.LevelAt(lineCurrent)) {
                styler.SetLevel(lineCurrent, level);
            }
            lineCurrent++;
            levelCurrent = levelNext;
            bOnlySpaces = true;
        }
        if (!isspacechar(ch)) {
            bOnlySpaces = false;
        }
    }

}

extern const LexerModule lmAHK2(SCLEX_AHK2, LexerAHK2::LexerFactoryAHK2, "ahk2", ahk2WordLists);

