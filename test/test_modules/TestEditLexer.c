#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "StyleLexers\StyleLexers.h"

SETTINGS2_T Settings2;
FOCUSEDVIEW_T FocusedView;
HANDLE g_hndlScintilla;

typedef struct Property {
    const char* key;
    const char* value;
} Property;

static int currentLexer;
static int startStyle;
static int endStyle;
static int unexpectedCalls;
static Property properties[16];
static int propertyCount;

LRESULT WINAPI Scintilla_DirectFunction(HANDLE handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    (void)handle;
    switch (message) {
    case SCI_GETLEXER: return currentLexer;
    case SCI_GETSELECTIONSTART: return 15;
    case SCI_GETSELECTIONEND: return 25;
    case SCI_LINEFROMPOSITION:
        if (wParam != 15) ++unexpectedCalls;
        return 2;
    case SCI_POSITIONFROMLINE:
        if (wParam != 2) ++unexpectedCalls;
        return 10;
    case SCI_GETSTYLEINDEXAT:
        if (wParam == 10) return startStyle;
        if (wParam == 25) return endStyle;
        ++unexpectedCalls;
        return 0;
    case SCI_SETPROPERTY:
        if (propertyCount < (int)ARRAYSIZE(properties)) {
            properties[propertyCount].key = (const char*)wParam;
            properties[propertyCount].value = (const char*)lParam;
        }
        ++propertyCount;
        return 0;
    default:
        ++unexpectedCalls;
        return 0;
    }
}

static int CheckComments(int lexer, int beginStyle, int finishStyle, LPCWSTR begin, LPCWSTR end, LPCWSTR line, bool atStart)
{
    WCHAR actualBegin[32] = L"before";
    WCHAR actualEnd[32] = L"before";
    WCHAR actualLine[32] = L"before";
    currentLexer = lexer;
    startStyle = beginStyle;
    endStyle = finishStyle;
    unexpectedCalls = 0;
    Lexer_GetStreamCommentStrgs(actualBegin, actualEnd, ARRAYSIZE(actualBegin));
    bool actualAtStart = Lexer_GetLineCommentStrg(actualLine, ARRAYSIZE(actualLine));
    if (unexpectedCalls || wcscmp(actualBegin, begin) || wcscmp(actualEnd, end) ||
        wcscmp(actualLine, line) || actualAtStart != atStart) {
        fprintf(stderr, "Unexpected comment behavior: lexer %d, styles %d/%d\n", lexer, beginStyle, finishStyle);
        return 1;
    }
    return 0;
}

static int CheckProperties(const Property* expected, int expectedCount)
{
    if (unexpectedCalls || propertyCount != expectedCount || propertyCount > (int)ARRAYSIZE(properties)) {
        fprintf(stderr, "Unexpected property count: %d\n", propertyCount);
        return 1;
    }
    for (int i = 0; i < expectedCount; ++i) {
        if (strcmp(properties[i].key, expected[i].key) || strcmp(properties[i].value, expected[i].value)) {
            fprintf(stderr, "Unexpected property %d\n", i);
            return 1;
        }
    }
    return 0;
}

static int CheckSpecificProperties(int lexer, const Property* expected, int expectedCount)
{
    unexpectedCalls = 0;
    propertyCount = 0;
    Lexer_SetLexerSpecificProperties(lexer);
    return CheckProperties(expected, expectedCount);
}

#define CHECK_PROPERTIES(lexer, list) CheckSpecificProperties((lexer), (list), (int)ARRAYSIZE(list))

int main(void)
{
    static const struct {
        int lexer;
        LPCWSTR begin;
        LPCWSTR end;
        LPCWSTR line;
        bool atStart;
    } cases[] = {
        { SCLEX_AU3, L"#cs", L"#ce", L";", false },
        { SCLEX_CPP, L"/*", L"*/", L"//", false },
        { SCLEX_D, L"/*", L"*/", L"//", false },
        { SCLEX_CSS, L"/*", L"*/", L"", false },
        { SCLEX_SQL, L"/*", L"*/", L"--", true },
        { SCLEX_PASCAL, L"{", L"}", L"//", false },
        { SCLEX_LUA, L"--[[", L"]]", L"--", true },
        { SCLEX_COFFEESCRIPT, L"###", L"###", L"#", true },
        { SCLEX_MATLAB, L"%{", L"%}", L"%", true },
        { SCLEX_PYTHON, L"_=\"\"\"", L"\"\"\"", L"#", false },
        { SCLEX_ASM, L"", L"", L";", true },
        { SCLEX_AHK, L"/*", L"*/", L";", false },
        { SCLEX_BATCH, L"", L"", L"rem ", false },
        { SCLEX_APDL, L"", L"", L"!", true },
        { SCLEX_VB, L"", L"", L"'", false },
        { SCLEX_NIM, L"", L"", L"#", true },
        { SCLEX_BASH, L"", L"", L"#", false },
        { SCLEX_XML, L"<!--", L"-->", L"", false },
        { SCLEX_NULL, L"", L"", L"", false }
    };
    for (int i = 0; i < (int)ARRAYSIZE(cases); ++i) {
        if (CheckComments(cases[i].lexer, SCE_H_DEFAULT, SCE_H_DEFAULT,
                          cases[i].begin, cases[i].end, cases[i].line, cases[i].atStart)) return 1;
    }
    if (CheckComments(SCLEX_HTML, SCE_HPHP_WORD, SCE_HPHP_OPERATOR, L"/*", L"*/", L"//", false) ||
        CheckComments(SCLEX_HTML, SCE_HJA_REGEX, SCE_HJ_START, L"/*", L"*/", L"//", false) ||
        CheckComments(SCLEX_HTML, SCE_HP_START, SCE_HP_IDENTIFIER, L"<!--", L"-->", L"#", false) ||
        CheckComments(SCLEX_HTML, SCE_HPA_IDENTIFIER, SCE_HPA_START, L"<!--", L"-->", L"#", false) ||
        CheckComments(SCLEX_HTML, SCE_HPHP_COMPLEX_VARIABLE, SCE_HPHP_COMPLEX_VARIABLE, L"<!--", L"-->", L"//", false) ||
        CheckComments(SCLEX_HTML, SCE_H_DEFAULT, SCE_HPHP_WORD, L"<!--", L"-->", L"", false) ||
        CheckComments(SCLEX_NULL, SCE_HP_START, SCE_HP_IDENTIFIER, L"", L"", L"#", false) ||
        CheckComments(SCLEX_CSS, SCE_HPHP_WORD, SCE_HPHP_OPERATOR, L"/*", L"*/", L"//", false)) return 1;

    WCHAR text[4] = L"old";
    Lexer_GetStreamCommentStrgs(NULL, text, ARRAYSIZE(text));
    Lexer_GetStreamCommentStrgs(text, NULL, ARRAYSIZE(text));
    Lexer_GetStreamCommentStrgs(text, text, 0);
    if (wcscmp(text, L"old") || Lexer_GetLineCommentStrg(NULL, 4) ||
        Lexer_GetLineCommentStrg(text, 0) || wcscmp(text, L"old")) return 1;
    currentLexer = SCLEX_CPP;
    Lexer_GetStreamCommentStrgs(text, text, ARRAYSIZE(text));
    if (wcscmp(text, L"*/")) return 1;

    EDITLEXER lexer = { 0 };
    static const int noFolding[] = { SCLEX_NULL, SCLEX_CONTAINER, SCLEX_BATCH, SCLEX_CONF, SCLEX_MAKEFILE, SCLEX_MARKDOWN };
    for (int i = 0; i < (int)ARRAYSIZE(noFolding); ++i) {
        lexer.lexerID = noFolding[i];
        FocusedView.CodeFoldingAvailable = true;
        Lexer_SetFoldingAvailability(&lexer);
        if (FocusedView.CodeFoldingAvailable) return 1;
    }
    lexer.lexerID = SCLEX_CPP;
    FocusedView.CodeFoldingAvailable = false;
    Lexer_SetFoldingAvailability(&lexer);
    if (!FocusedView.CodeFoldingAvailable) return 1;

    static const Property folding[] = {
        { "fold", "1" }, { "fold.comment", "1" }, { "fold.compact", "0" },
        { "fold.foldsyntaxbased", "1" }, { "fold.html", "1" },
        { "fold.preprocessor", "1" }, { "fold.cpp.comment.explicit", "0" }
    };
    static const Property off[] = { { "fold", "0" } };
    static const Property focused[] = { { "fold", "0" }, { "fold.comment", "1" }, { "fold.compact", "0" } };
    unexpectedCalls = propertyCount = 0;
    Lexer_SetFoldingProperties(true);
    if (CheckProperties(folding, (int)ARRAYSIZE(folding))) return 1;
    propertyCount = 0;
    Lexer_SetFoldingProperties(false);
    if (CheckProperties(off, (int)ARRAYSIZE(off))) return 1;
    propertyCount = 0;
    Lexer_SetFoldingFocusedView();
    if (CheckProperties(focused, (int)ARRAYSIZE(focused))) return 1;

    static const Property cpp[] = { { "styling.within.preprocessor", "1" }, { "lexer.cpp.track.preprocessor", "0" }, { "lexer.cpp.update.preprocessor", "0" } };
    static const Property pascalProps[] = { { "lexer.pascal.smart.highlighting", "1" } };
    static const Property sql[] = { { "fold.sql.at.else", "1" }, { "sql.backslash.escapes", "1" }, { "lexer.sql.backticks.identifier", "1" }, { "lexer.sql.numbersign.comment", "0" } };
    static const Property sqlHash[] = { { "fold.sql.at.else", "1" }, { "sql.backslash.escapes", "1" }, { "lexer.sql.backticks.identifier", "1" }, { "lexer.sql.numbersign.comment", "1" } };
    static const Property markdown[] = { { "lexer.markdown.header.eolfill", "1" } };
    static const Property nsis[] = { { "nsis.ignorecase", "1" } };
    static const Property css[] = { { "lexer.css.scss.language", "1" }, { "lexer.css.less.language", "1" } };
    static const Property json[] = { { "lexer.json.escape.sequence", "1" } };
    static const Property json5[] = { { "lexer.json5.allow.comments", "1" }, { "lexer.json5.escape.sequence", "1" } };
    static const Property python[] = {
        { "tab.timmy.whinge.level", "1" }, { "lexer.python.strings.f", "1" },
        { "lexer.python.literals.binary", "1" }, { "lexer.python.strings.u", "1" },
        { "lexer.python.strings.b", "1" }, { "lexer.python.strings.over.newline", "0" },
        { "lexer.python.unicode.identifiers", "1" }, { "lexer.python.identifier.attributes", "0" }
    };
    static const Property asmProps[] = {
        { "lexer.asm.comment.delimiter", ";" }, { "fold.asm.syntax.based", "1" },
        { "fold.asm.comment.multiline", "1" }, { "fold.asm.comment.explicit", "0" },
        { "fold.asm.explicit.start", ";{" }, { "fold.asm.explicit.end", ";}" }, { "fold.asm.explicit.anywhere", "0" }
    };
    static const Property bash[] = {
        { "lexer.bash.styling.inside.string", "1" }, { "lexer.bash.styling.inside.backticks", "1" },
        { "lexer.bash.styling.inside.parameter", "1" }, { "lexer.bash.styling.inside.heredoc", "1" },
        { "lexer.bash.command.substitution", "1" }, { "lexer.bash.nested.backticks", "1" },
        { "lexer.bash.special.parameter", "1" }
    };
    static const Property d[] = {
        { "fold.d.syntax.based", "1" }, { "fold.d.comment.multiline", "1" }, { "fold.d.comment.explicit", "0" },
        { "fold.d.explicit.start", "//{" }, { "fold.d.explicit.end", "//}" }, { "fold.d.explicit.anywhere", "0" }, { "lexer.d.fold.at.else", "1" }
    };
    static const Property julia[] = {
        { "fold.julia.docstring", "1" }, { "fold.julia.syntax.based", "1" },
        { "lexer.julia.highlight.typeannotation", "1" }, { "lexer.julia.highlight.lexerror", "1" }
    };
    static const Property kotlin[] = { { "fold", "1" }, { "fold.comment", "1" } };
    static const Property nim[] = { { "lexer.nim.raw.strings.highlight.ident", "1" } };
    static const Property verilog[] = {
        { "lexer.verilog.track.preprocessor", "1" }, { "lexer.verilog.update.preprocessor", "1" },
        { "lexer.verilog.portstyling", "1" }, { "lexer.verilog.allupperkeywords", "1" }
    };
    static const Property xml[] = { { "lexer.xml.allow.scripts", "1" } };
    Settings2.LexerSQLNumberSignAsComment = false;
    if (CHECK_PROPERTIES(SCLEX_CPP, cpp) || CHECK_PROPERTIES(SCLEX_PASCAL, pascalProps) ||
        CHECK_PROPERTIES(SCLEX_SQL, sql)) return 1;
    Settings2.LexerSQLNumberSignAsComment = true;
    if (CHECK_PROPERTIES(SCLEX_SQL, sqlHash) || CHECK_PROPERTIES(SCLEX_MARKDOWN, markdown) ||
        CHECK_PROPERTIES(SCLEX_NSIS, nsis) || CHECK_PROPERTIES(SCLEX_CSS, css) ||
        CHECK_PROPERTIES(SCLEX_JSON, json) || CHECK_PROPERTIES(SCLEX_JSON5, json5) ||
        CHECK_PROPERTIES(SCLEX_PYTHON, python) || CHECK_PROPERTIES(SCLEX_ASM, asmProps) ||
        CHECK_PROPERTIES(SCLEX_BASH, bash) || CHECK_PROPERTIES(SCLEX_D, d) ||
        CHECK_PROPERTIES(SCLEX_JULIA, julia) || CHECK_PROPERTIES(SCLEX_KOTLIN, kotlin) ||
        CHECK_PROPERTIES(SCLEX_NIM, nim) || CHECK_PROPERTIES(SCLEX_VERILOG, verilog) ||
        CHECK_PROPERTIES(SCLEX_SYSVERILOG, verilog) || CHECK_PROPERTIES(SCLEX_XML, xml) ||
        CheckSpecificProperties(SCLEX_NULL, NULL, 0)) return 1;

    puts("Comment, folding and lexer-property behavior verified");
    return 0;
}
