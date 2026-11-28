#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "StyleLexers\StyleLexers.h"

#define COUNTOF(value) ARRAYSIZE(value)

typedef struct StyleCall {
    int style;
    LPCWSTR value;
    float fontSize;
} StyleCall;

typedef struct ExpectedCall {
    int style;
    int row;
} ExpectedCall;

static StyleCall calls[512];
static int callCount;

static bool IsLexerStandard(PEDITLEXER lexer)
{
    return lexer && (lexer == &lexStandard || lexer == &lexStandard2nd);
}

static float Style_GetBaseFontSize(void)
{
    return 10.0f;
}

static bool Style_StrGetSizeFloatEx(LPCWSTR value, float* size)
{
    if (wcscmp(value, L"size:17") == 0) {
        *size = 17.0f;
        return true;
    }
    return false;
}

static void Style_SetStyles(HWND hwnd, const int style, LPCWSTR value, const float fontSize)
{
    (void)hwnd;
    if (callCount < (int)COUNTOF(calls)) {
        calls[callCount].style = style;
        calls[callCount].value = value;
        calls[callCount].fontSize = fontSize;
    }
    ++callCount;
}

#include "StyleFillRelatedStyles.inc"

static int CaptureLexer(PEDITLEXER lexer, int lexerID, const char* lexerName, LPCWSTR name, const char* keyword, int rowCount)
{
    if (lexer->lexerID != lexerID || strcmp(lexer->lexerName, lexerName) != 0 ||
        wcscmp(lexer->pszName, name) != 0 || !lexer->pKeyWords ||
        (keyword ? (!lexer->pKeyWords->pszKeyWords[0] || !strstr(lexer->pKeyWords->pszKeyWords[0], keyword))
                 : lexer->pKeyWords->pszKeyWords[0] != NULL)) {
        fputs("Unexpected lexer metadata\n", stderr);
        return 1;
    }
    for (int i = 0; i < rowCount; ++i) {
        if (lexer->Styles[i].iStyle == -1 || !lexer->Styles[i].pszName ||
            !lexer->Styles[i].pszDefault || lexer->Styles[i].szValue[0] != L'\0') {
            fprintf(stderr, "Unexpected style row %d\n", i);
            return 1;
        }
        lexer->Styles[i].szValue[0] = (WCHAR)(L'A' + i);
        lexer->Styles[i].szValue[1] = L'\0';
    }
    if (lexer->Styles[rowCount].iStyle != -1) {
        fputs("Missing sentinel\n", stderr);
        return 1;
    }
    wcscpy_s(lexer->Styles[0].szValue, COUNTOF(lexer->Styles[0].szValue), L"size:17");

    callCount = 0;
    Style_FillRelatedStyles(NULL, lexer);
    if (callCount > (int)COUNTOF(calls)) {
        fputs("Too many style calls\n", stderr);
        return 1;
    }
    return 0;
}

static int CheckLexer(PEDITLEXER lexer, int lexerID, const char* lexerName, LPCWSTR name, const char* keyword, int rowCount,
    const ExpectedCall* expected, int expectedCount)
{
    if (CaptureLexer(lexer, lexerID, lexerName, name, keyword, rowCount)) {
        return 1;
    }
    if (callCount != expectedCount) {
        fprintf(stderr, "Expected %d style calls, got %d\n", expectedCount, callCount);
        return 1;
    }
    for (int i = 0; i < callCount; ++i) {
        if (calls[i].style != expected[i].style ||
            calls[i].value != lexer->Styles[expected[i].row].szValue ||
            calls[i].fontSize != 17.0f) {
            fprintf(stderr, "Unexpected style call %d\n", i);
            return 1;
        }
    }
    printf("%ls: %d style rows and %d style calls verified\n", name, rowCount, callCount);
    return 0;
}

static int CheckWebLexer(PEDITLEXER lexer, int lexerID, const char* lexerName, LPCWSTR name, const char* keyword, int rowCount,
    const ExpectedCall* witnesses, int witnessCount, int expectedCount, unsigned int expectedHash)
{
    if (CaptureLexer(lexer, lexerID, lexerName, name, keyword, rowCount)) {
        return 1;
    }
    // Snapshot the ordered style/row pairs, independent of pointer values.
    unsigned int hash = 2166136261u;
    for (int i = 0; i < callCount; ++i) {
        int row = 0;
        while (row < rowCount && calls[i].value != lexer->Styles[row].szValue) {
            ++row;
        }
        if (row == rowCount || calls[i].fontSize != 17.0f) {
            fprintf(stderr, "Unexpected web style call %d\n", i);
            return 1;
        }
        hash = (hash ^ (unsigned int)calls[i].style) * 16777619u;
        hash = (hash ^ (unsigned int)row) * 16777619u;
    }
    for (int i = 0; i < witnessCount; ++i) {
        int j = 0;
        while (j < callCount && (calls[j].style != witnesses[i].style ||
               calls[j].value != lexer->Styles[witnesses[i].row].szValue)) {
            ++j;
        }
        if (j == callCount) {
            fprintf(stderr, "Missing web style %d for row %d\n", witnesses[i].style, witnesses[i].row);
            return 1;
        }
    }
    if (lexerID == SCLEX_XML) {
        int pythonCalls = 0;
        int aspPythonCalls = 0;
        for (int i = 0; i < callCount; ++i) {
            if (calls[i].value == lexer->Styles[11].szValue) {
                pythonCalls += (calls[i].style == SCE_HP_WORD);
                aspPythonCalls += (calls[i].style == SCE_HPA_IDENTIFIER);
            }
        }
        if (pythonCalls != 2 || aspPythonCalls != 2) {
            fputs("Unexpected XML CDATA Python dispatch multiplicity\n", stderr);
            return 1;
        }
    }
    printf("%ls: %d style rows, %d calls, hash %08x\n", name, rowCount, callCount, hash);
    if (callCount != expectedCount || hash != expectedHash) {
        fputs("Unexpected web dispatch sequence\n", stderr);
        return 1;
    }
    return 0;
}

static int CheckDefaultFontSize(PEDITLEXER lexer, float expected)
{
    lexer->Styles[0].szValue[0] = L'\0';
    callCount = 0;
    Style_FillRelatedStyles(NULL, lexer);
    if (callCount == 0 || callCount > (int)COUNTOF(calls)) {
        fputs("Unexpected standard lexer call count\n", stderr);
        return 1;
    }
    for (int i = 0; i < callCount; ++i) {
        if (calls[i].fontSize != expected) {
            fputs("Unexpected standard lexer base font size\n", stderr);
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    static const ExpectedCall cppExpected[] = {
        { SCE_C_IDENTIFIER, 1 },
        { SCE_C_COMMENT, 2 }, { SCE_C_COMMENTLINE, 2 },
        { SCE_C_WORD, 3 },
        { SCE_C_WORD2, 4 },
        { SCE_C_GLOBALCLASS, 5 },
        { SCE_C_STRING, 6 }, { SCE_C_CHARACTER, 6 }, { SCE_C_STRINGEOL, 6 },
        { SCE_C_VERBATIM, 6 }, { SCE_C_STRINGRAW, 6 }, { SCE_C_HASHQUOTEDSTRING, 6 },
        { SCE_C_NUMBER, 7 },
        { SCE_C_OPERATOR, 8 },
        { SCE_C_PREPROCESSOR, 9 }, { SCE_C_PREPROCESSORCOMMENT, 9 },
        { SCE_C_PREPROCESSORCOMMENTDOC, 9 },
        { SCE_C_VERBATIM, 10 }, { SCE_C_TRIPLEVERBATIM, 10 },
        { SCE_C_COMMENTDOC, 11 }, { SCE_C_COMMENTLINEDOC, 11 },
        { SCE_C_COMMENTDOCKEYWORD, 12 },
        { SCE_C_COMMENTDOCKEYWORDERROR, 13 },
        { SCE_C_TASKMARKER, 14 },
        { SCE_C_USERLITERAL, 15 }
    };
    static const ExpectedCall javaExpected[] = {
        { SCE_C_IDENTIFIER, 1 },
        { SCE_C_COMMENT, 2 }, { SCE_C_COMMENTLINE, 2 },
        { SCE_C_WORD, 3 }, { SCE_C_WORD2, 4 },
        { SCE_C_STRING, 5 }, { SCE_C_CHARACTER, 5 }, { SCE_C_STRINGEOL, 5 },
        { SCE_C_VERBATIM, 5 }, { SCE_C_STRINGRAW, 5 }, { SCE_C_HASHQUOTEDSTRING, 5 },
        { SCE_C_NUMBER, 6 }, { SCE_C_OPERATOR, 7 },
        { SCE_C_COMMENTDOC, 8 }, { SCE_C_COMMENTLINEDOC, 8 },
        { SCE_C_COMMENTDOCKEYWORD, 9 }, { SCE_C_COMMENTDOCKEYWORDERROR, 10 },
        { SCE_C_TASKMARKER, 11 }
    };
    static const ExpectedCall jsExpected[] = {
        { SCE_C_IDENTIFIER, 1 },
        { SCE_C_COMMENT, 2 }, { SCE_C_COMMENTLINE, 2 },
        { SCE_C_WORD, 3 },
        { SCE_C_STRING, 4 }, { SCE_C_CHARACTER, 4 }, { SCE_C_STRINGEOL, 4 },
        { SCE_C_VERBATIM, 4 }, { SCE_C_STRINGRAW, 4 }, { SCE_C_HASHQUOTEDSTRING, 4 },
        { SCE_C_REGEX, 5 }, { SCE_C_NUMBER, 6 }, { SCE_C_OPERATOR, 7 },
        { SCE_C_COMMENTDOC, 8 }, { SCE_C_COMMENTLINEDOC, 8 },
        { SCE_C_COMMENTDOCKEYWORD, 9 }, { SCE_C_COMMENTDOCKEYWORDERROR, 10 },
        { SCE_C_TASKMARKER, 11 }
    };
    static const ExpectedCall rcExpected[] = {
        { SCE_C_IDENTIFIER, 1 },
        { SCE_C_COMMENT, 2 }, { SCE_C_COMMENTLINE, 2 },
        { SCE_C_WORD, 3 },
        { SCE_C_STRING, 4 }, { SCE_C_CHARACTER, 4 }, { SCE_C_STRINGEOL, 4 },
        { SCE_C_VERBATIM, 4 }, { SCE_C_STRINGRAW, 4 }, { SCE_C_HASHQUOTEDSTRING, 4 },
        { SCE_C_NUMBER, 5 }, { SCE_C_OPERATOR, 6 },
        { SCE_C_PREPROCESSOR, 7 }, { SCE_C_PREPROCESSORCOMMENT, 7 },
        { SCE_C_PREPROCESSORCOMMENTDOC, 7 },
        { SCE_C_COMMENTDOC, 8 }, { SCE_C_COMMENTLINEDOC, 8 },
        { SCE_C_COMMENTDOCKEYWORD, 9 }, { SCE_C_COMMENTDOCKEYWORDERROR, 10 },
        { SCE_C_TASKMARKER, 11 }
    };
    static const ExpectedCall dExpected[] = {
        { SCE_D_COMMENT, 1 }, { SCE_D_COMMENTLINE, 1 }, { SCE_D_COMMENTNESTED, 1 },
        { SCE_D_COMMENTDOC, 2 }, { SCE_D_NUMBER, 3 },
        { SCE_D_WORD, 4 }, { SCE_D_WORD2, 5 }, { SCE_D_TYPEDEF, 6 },
        { SCE_D_STRING, 7 }, { SCE_D_CHARACTER, 7 }, { SCE_D_STRINGEOL, 7 },
        { SCE_D_OPERATOR, 8 }, { SCE_D_IDENTIFIER, 9 }
    };
    static const ExpectedCall goExpected[] = {
        { SCE_D_COMMENT, 1 }, { SCE_D_COMMENTLINE, 1 }, { SCE_D_COMMENTNESTED, 1 },
        { SCE_D_NUMBER, 2 }, { SCE_D_WORD, 3 }, { SCE_D_WORD2, 4 }, { SCE_D_TYPEDEF, 5 },
        { SCE_D_STRING, 6 }, { SCE_D_CHARACTER, 6 }, { SCE_D_STRINGEOL, 6 },
        { SCE_D_OPERATOR, 7 }, { SCE_D_IDENTIFIER, 8 }
    };
    static const ExpectedCall pythonExpected[] = {
        { SCE_P_COMMENTLINE, 1 }, { SCE_P_COMMENTBLOCK, 1 },
        { SCE_P_WORD, 2 }, { SCE_P_WORD2, 3 }, { SCE_P_IDENTIFIER, 4 }, { SCE_P_ATTRIBUTE, 5 },
        { SCE_P_CHARACTER, 6 }, { SCE_P_FCHARACTER, 6 },
        { SCE_P_STRING, 7 }, { SCE_P_FSTRING, 7 }, { SCE_P_STRINGEOL, 7 },
        { SCE_P_TRIPLE, 8 }, { SCE_P_FTRIPLE, 8 },
        { SCE_P_TRIPLEDOUBLE, 9 }, { SCE_P_FTRIPLEDOUBLE, 9 },
        { SCE_P_DECORATOR, 10 }, { SCE_P_NUMBER, 11 }, { SCE_P_OPERATOR, 12 },
        { SCE_P_DEFNAME, 13 }, { SCE_P_CLASSNAME, 14 }
    };
    static const ExpectedCall awkExpected[] = {
        { SCE_P_WORD, 1 }, { SCE_P_WORD2, 2 }, { SCE_P_IDENTIFIER, 3 },
        { SCE_P_COMMENTLINE, 4 }, { SCE_P_COMMENTBLOCK, 4 },
        { SCE_P_STRING, 5 }, { SCE_P_STRINGEOL, 5 }, { SCE_P_CHARACTER, 5 },
        { SCE_P_NUMBER, 6 }, { SCE_P_OPERATOR, 7 }
    };
    static const ExpectedCall jsonExpected[] = {
        { SCE_JSON_LINECOMMENT, 1 }, { SCE_JSON_BLOCKCOMMENT, 1 },
        { SCE_JSON_KEYWORD, 2 }, { SCE_JSON_LDKEYWORD, 3 },
        { SCE_JSON_STRING, 4 }, { SCE_JSON_STRINGEOL, 5 },
        { SCE_JSON_NUMBER, 6 }, { SCE_JSON_OPERATOR, 7 },
        { SCE_JSON_PROPERTYNAME, 8 }, { SCE_JSON_ESCAPESEQUENCE, 9 },
        { SCE_JSON_URI, 10 }, { SCE_JSON_COMPACTIRI, 11 }, { SCE_JSON_ERROR, 12 }
    };
    static const ExpectedCall json5Expected[] = {
        { SCE_JSON5_LINECOMMENT, 1 }, { SCE_JSON5_BLOCKCOMMENT, 1 },
        { SCE_JSON5_KEYWORD, 2 }, { SCE_JSON5_LDKEYWORD, 3 },
        { SCE_JSON5_STRING, 4 }, { SCE_JSON5_STRINGEOL, 5 },
        { SCE_JSON5_NUMBER, 6 }, { SCE_JSON5_OPERATOR, 7 },
        { SCE_JSON5_PROPERTYNAME, 8 }, { SCE_JSON5_ESCAPESEQUENCE, 9 },
        { SCE_JSON5_URI, 10 }, { SCE_JSON5_COMPACTIRI, 11 }, { SCE_JSON5_ERROR, 12 }
    };
    static const ExpectedCall yamlExpected[] = {
        { SCE_YAML_COMMENT, 1 }, { SCE_YAML_IDENTIFIER, 2 }, { SCE_YAML_KEYWORD, 3 },
        { SCE_YAML_NUMBER, 4 }, { SCE_YAML_REFERENCE, 5 }, { SCE_YAML_DOCUMENT, 6 },
        { SCE_YAML_TEXT, 7 }, { SCE_YAML_ERROR, 8 }, { SCE_YAML_OPERATOR, 9 }
    };
    static const ExpectedCall tomlExpected[] = {
        { SCE_TOML_IDENTIFIER, 1 }, { SCE_TOML_KEYWORD, 2 }, { SCE_TOML_COMMENT, 3 },
        { SCE_TOML_KEY, 4 }, { SCE_TOML_OPERATOR, 5 }, { SCE_TOML_NUMBER, 6 },
        { SCE_TOML_DATETIME, 7 }, { SCE_TOML_TABLE, 8 },
        { SCE_TOML_STRING_SQ, 9 }, { SCE_TOML_TRIPLE_STRING_SQ, 9 },
        { SCE_TOML_STRING_DQ, 10 }, { SCE_TOML_TRIPLE_STRING_DQ, 10 },
        { SCE_TOML_STRINGEOL, 11 }, { SCE_TOML_ESCAPECHAR, 12 }, { SCE_TOML_ERROR, 13 }
    };
    static const ExpectedCall vbExpected[] = {
        { SCE_B_COMMENT, 1 }, { SCE_B_KEYWORD, 2 }, { SCE_B_IDENTIFIER, 3 },
        { SCE_B_STRING, 4 }, { SCE_B_STRINGEOL, 4 },
        { SCE_B_NUMBER, 5 }, { SCE_B_DATE, 5 },
        { SCE_B_OPERATOR, 6 }, { SCE_B_PREPROCESSOR, 7 }
    };
    static const ExpectedCall vbsExpected[] = {
        { SCE_B_COMMENT, 1 }, { SCE_B_KEYWORD, 2 }, { SCE_B_IDENTIFIER, 3 },
        { SCE_B_STRING, 4 }, { SCE_B_STRINGEOL, 4 },
        { SCE_B_NUMBER, 5 }, { SCE_B_OPERATOR, 6 }
    };
    static const ExpectedCall bashExpected[] = {
        { SCE_SH_ERROR, 1 }, { SCE_SH_COMMENTLINE, 2 }, { SCE_SH_NUMBER, 3 },
        { SCE_SH_WORD, 4 }, { SCE_SH_STRING, 5 }, { SCE_SH_CHARACTER, 6 },
        { SCE_SH_OPERATOR, 7 }, { SCE_SH_IDENTIFIER, 8 }, { SCE_SH_SCALAR, 9 },
        { SCE_SH_PARAM, 10 }, { SCE_SH_BACKTICKS, 11 },
        { SCE_SH_HERE_DELIM, 12 }, { SCE_SH_HERE_Q, 13 }
    };
    static const ExpectedCall batExpected[] = {
        { SCE_BAT_COMMENT, 1 }, { SCE_BAT_WORD, 2 }, { SCE_BAT_IDENTIFIER, 3 },
        { SCE_BAT_OPERATOR, 4 }, { SCE_BAT_COMMAND, 5 }, { SCE_BAT_HIDE, 5 },
        { SCE_BAT_LABEL, 6 }, { SCE_BAT_AFTER_LABEL, 7 },
        { SCE_B_DOCLINE, 8 }, { SCE_B_DOCBLOCK, 8 }
    };
    static const ExpectedCall psExpected[] = {
        { SCE_POWERSHELL_COMMENT, 1 }, { SCE_POWERSHELL_COMMENTSTREAM, 1 },
        { SCE_POWERSHELL_COMMENTDOCKEYWORD, 1 }, { SCE_POWERSHELL_KEYWORD, 2 },
        { SCE_POWERSHELL_IDENTIFIER, 3 },
        { SCE_POWERSHELL_STRING, 4 }, { SCE_POWERSHELL_CHARACTER, 4 },
        { SCE_POWERSHELL_NUMBER, 5 }, { SCE_POWERSHELL_OPERATOR, 6 },
        { SCE_POWERSHELL_VARIABLE, 7 },
        { SCE_POWERSHELL_CMDLET, 8 }, { SCE_POWERSHELL_FUNCTION, 8 },
        { SCE_POWERSHELL_ALIAS, 9 },
        { SCE_POWERSHELL_HERE_STRING, 10 }, { SCE_POWERSHELL_HERE_CHARACTER, 10 }
    };
    static const ExpectedCall rubyExpected[] = {
        { SCE_RB_COMMENTLINE, 1 }, { SCE_P_COMMENTBLOCK, 1 },
        { SCE_RB_WORD, 2 }, { SCE_RB_IDENTIFIER, 3 },
        { SCE_RB_NUMBER, 4 }, { SCE_RB_OPERATOR, 5 },
        { SCE_RB_STRING, 6 }, { SCE_RB_CHARACTER, 6 }, { SCE_P_STRINGEOL, 6 },
        { SCE_RB_CLASSNAME, 7 }, { SCE_RB_DEFNAME, 8 }, { SCE_RB_POD, 9 },
        { SCE_RB_REGEX, 10 }, { SCE_RB_GLOBAL, 11 }, { SCE_RB_SYMBOL, 12 },
        { SCE_RB_MODULE_NAME, 13 }, { SCE_RB_INSTANCE_VAR, 14 }, { SCE_RB_CLASS_VAR, 15 },
        { SCE_RB_DATASECTION, 16 }, { SCE_RB_ERROR, 17 }, { SCE_RB_BACKTICKS, 18 },
        { SCE_RB_HERE_DELIM, 19 }, { SCE_RB_HERE_Q, 19 }, { SCE_RB_HERE_QQ, 19 }, { SCE_RB_HERE_QX, 19 },
        { SCE_RB_STRING_Q, 20 }, { SCE_RB_STRING_QQ, 20 }, { SCE_RB_STRING_QX, 20 }, { SCE_RB_STRING_QR, 20 },
        { SCE_RB_STRING_QW, 21 }, { SCE_RB_STRING_W, 21 }, { SCE_RB_STRING_I, 21 }, { SCE_RB_STRING_QI, 21 },
        { SCE_RB_STRING_QS, 22 }, { SCE_RB_WORD_DEMOTED, 23 },
        { SCE_RB_STDIN, 24 }, { SCE_RB_STDOUT, 24 }, { SCE_RB_STDERR, 24 }
    };
    static const ExpectedCall perlExpected[] = {
        { SCE_PL_COMMENTLINE, 1 }, { SCE_PL_WORD, 2 }, { SCE_PL_IDENTIFIER, 3 },
        { SCE_PL_STRING, 4 }, { SCE_PL_CHARACTER, 5 }, { SCE_PL_NUMBER, 6 },
        { SCE_PL_OPERATOR, 7 }, { SCE_PL_SCALAR, 8 }, { SCE_PL_ARRAY, 9 },
        { SCE_PL_HASH, 10 }, { SCE_PL_SYMBOLTABLE, 11 }, { SCE_PL_REGEX, 12 },
        { SCE_PL_REGSUBST, 13 }, { SCE_PL_BACKTICKS, 14 }, { SCE_PL_HERE_DELIM, 15 },
        { SCE_PL_HERE_Q, 16 }, { SCE_PL_HERE_QQ, 17 }, { SCE_PL_HERE_QX, 18 },
        { SCE_PL_STRING_Q, 19 }, { SCE_PL_STRING_QQ, 20 }, { SCE_PL_STRING_QX, 21 },
        { SCE_PL_STRING_QR, 22 }, { SCE_PL_STRING_QW, 23 }, { SCE_PL_SUB_PROTOTYPE, 24 },
        { SCE_PL_FORMAT_IDENT, 25 }, { SCE_PL_FORMAT, 26 }, { SCE_PL_POD, 27 },
        { SCE_PL_POD_VERB, 28 }, { SCE_PL_DATASECTION, 29 }, { SCE_PL_ERROR, 30 },
        { SCE_PL_XLAT, 31 },
        { SCE_PL_REGEX_VAR, 32 }, { SCE_PL_REGSUBST_VAR, 32 }, { SCE_PL_BACKTICKS_VAR, 32 },
        { SCE_PL_HERE_QQ_VAR, 33 }, { SCE_PL_HERE_QX_VAR, 33 },
        { SCE_PL_STRING_QQ_VAR, 33 }, { SCE_PL_STRING_QX_VAR, 33 }
    };
    static const ExpectedCall luaExpected[] = {
        { SCE_LUA_COMMENT, 1 }, { SCE_LUA_COMMENTLINE, 1 }, { SCE_LUA_COMMENTDOC, 1 },
        { SCE_LUA_NUMBER, 2 }, { SCE_LUA_WORD, 3 }, { SCE_LUA_WORD2, 4 },
        { SCE_LUA_WORD3, 5 }, { SCE_LUA_WORD4, 6 },
        { SCE_LUA_STRING, 7 }, { SCE_LUA_STRINGEOL, 7 }, { SCE_LUA_CHARACTER, 7 },
        { SCE_LUA_LITERALSTRING, 8 }, { SCE_LUA_PREPROCESSOR, 9 },
        { SCE_LUA_OPERATOR, 10 }, { SCE_LUA_IDENTIFIER, 11 }, { SCE_LUA_LABEL, 12 }
    };
    static const ExpectedCall rExpected[] = {
        { SCE_R_COMMENT, 1 }, { SCE_R_KWORD, 2 }, { SCE_R_BASEKWORD, 3 },
        { SCE_R_OTHERKWORD, 4 }, { SCE_R_NUMBER, 5 },
        { SCE_R_STRING, 6 }, { SCE_R_STRING2, 6 },
        { SCE_R_OPERATOR, 7 }, { SCE_R_IDENTIFIER, 8 }, { SCE_R_INFIX, 9 },
        { SCE_R_INFIXEOL, 10 }, { SCE_R_BACKTICKS, 11 },
        { SCE_R_RAWSTRING, 12 }, { SCE_R_RAWSTRING2, 12 }, { SCE_R_ESCAPESEQUENCE, 13 }
    };
    static const ExpectedCall matlabExpected[] = {
        { SCE_MATLAB_COMMENT, 1 }, { SCE_MATLAB_COMMAND, 2 },
        { SCE_MATLAB_NUMBER, 3 }, { SCE_MATLAB_KEYWORD, 4 },
        { SCE_MATLAB_STRING, 5 }, { SCE_MATLAB_DOUBLEQUOTESTRING, 5 },
        { SCE_MATLAB_OPERATOR, 6 }, { SCE_MATLAB_IDENTIFIER, 7 }
    };
    static const ExpectedCall rustExpected[] = {
        { SCE_RUST_IDENTIFIER, 1 }, { SCE_RUST_WORD, 2 }, { SCE_RUST_WORD2, 3 },
        { SCE_RUST_WORD3, 4 }, { SCE_RUST_NUMBER, 5 },
        { SCE_RUST_COMMENTBLOCK, 6 }, { SCE_RUST_COMMENTLINE, 6 },
        { SCE_RUST_COMMENTBLOCKDOC, 6 }, { SCE_RUST_COMMENTLINEDOC, 6 },
        { SCE_RUST_STRING, 7 }, { SCE_RUST_STRINGR, 7 }, { SCE_RUST_CHARACTER, 7 },
        { SCE_RUST_OPERATOR, 8 }, { SCE_RUST_MACRO, 9 }, { SCE_RUST_LIFETIME, 10 },
        { SCE_RUST_LEXERROR, 11 },
        { SCE_RUST_BYTESTRING, 12 }, { SCE_RUST_BYTESTRINGR, 12 }, { SCE_RUST_BYTECHARACTER, 12 },
        { SCE_RUST_CSTRING, 13 }, { SCE_RUST_CSTRINGR, 13 }
    };
    static const ExpectedCall kotlinExpected[] = {
        { SCE_KOTLIN_WORD, 1 }, { SCE_KOTLIN_ANNOTATION, 2 }, { SCE_KOTLIN_CLASS, 3 },
        { SCE_KOTLIN_INTERFACE, 4 }, { SCE_KOTLIN_ENUM, 5 },
        { SCE_KOTLIN_FUNCTION, 6 }, { SCE_KOTLIN_FUNCTION_DEFINITION, 6 },
        { SCE_KOTLIN_COMMENTBLOCK, 7 }, { SCE_KOTLIN_COMMENTLINE, 7 },
        { SCE_KOTLIN_COMMENTBLOCKDOC, 8 }, { SCE_KOTLIN_COMMENTLINEDOC, 8 },
        { SCE_KOTLIN_COMMENTDOCWORD, 9 }, { SCE_KOTLIN_TASKMARKER, 10 },
        { SCE_KOTLIN_CHARACTER, 11 }, { SCE_KOTLIN_STRING, 11 },
        { SCE_KOTLIN_RAWSTRING, 12 }, { SCE_KOTLIN_ESCAPECHAR, 13 },
        { SCE_KOTLIN_BACKTICKS, 14 }, { SCE_KOTLIN_LABEL, 15 },
        { SCE_KOTLIN_NUMBER, 16 }, { SCE_KOTLIN_VARIABLE, 17 },
        { SCE_KOTLIN_OPERATOR, 18 }, { SCE_KOTLIN_OPERATOR2, 18 },
        { SCE_KOTLIN_IDENTIFIER, 19 }
    };
    static const ExpectedCall dartExpected[] = {
        { SCE_DART_KW_PRIMARY, 1 },
        { SCE_DART_IDENTIFIER, 2 }, { SCE_DART_IDENTIFIER_STRING, 2 },
        { SCE_DART_KW_SECONDARY, 3 }, { SCE_DART_KW_TERTIARY, 3 }, { SCE_DART_KW_TYPE, 3 },
        { SCE_DART_METADATA, 4 },
        { SCE_DART_COMMENTBLOCK, 5 }, { SCE_DART_COMMENTLINE, 5 },
        { SCE_DART_COMMENTBLOCKDOC, 6 }, { SCE_DART_COMMENTLINEDOC, 6 },
        { SCE_DART_STRING_SQ, 7 }, { SCE_DART_STRING_DQ, 7 },
        { SCE_DART_TRIPLE_STRING_SQ, 8 }, { SCE_DART_TRIPLE_STRING_DQ, 8 },
        { SCE_DART_RAWSTRING_SQ, 9 }, { SCE_DART_RAWSTRING_DQ, 9 },
        { SCE_DART_TRIPLE_RAWSTRING_SQ, 9 }, { SCE_DART_TRIPLE_RAWSTRING_DQ, 9 },
        { SCE_DART_ESCAPECHAR, 10 }, { SCE_DART_NUMBER, 11 },
        { SCE_DART_OPERATOR, 12 }, { SCE_DART_OPERATOR_STRING, 12 },
        { SCE_DART_SYMBOL_IDENTIFIER, 13 }, { SCE_DART_SYMBOL_OPERATOR, 14 }
    };
    static const ExpectedCall juliaExpected[] = {
        { SCE_JULIA_COMMENT, 1 }, { SCE_JULIA_NUMBER, 2 }, { SCE_JULIA_KEYWORD1, 3 },
        { SCE_JULIA_KEYWORD2, 4 }, { SCE_JULIA_KEYWORD3, 4 }, { SCE_JULIA_KEYWORD4, 4 },
        { SCE_JULIA_CHAR, 5 }, { SCE_JULIA_STRING, 5 },
        { SCE_JULIA_OPERATOR, 6 }, { SCE_JULIA_BRACKET, 6 },
        { SCE_JULIA_TYPEOPERATOR, 7 }, { SCE_JULIA_IDENTIFIER, 8 },
        { SCE_JULIA_SYMBOL, 9 }, { SCE_JULIA_MACRO, 10 }, { SCE_JULIA_DOCSTRING, 11 },
        { SCE_JULIA_STRINGINTERP, 12 }, { SCE_JULIA_STRINGLITERAL, 12 },
        { SCE_JULIA_COMMAND, 13 }, { SCE_JULIA_COMMANDLITERAL, 13 },
        { SCE_JULIA_TYPEANNOT, 14 }, { SCE_JULIA_LEXERROR, 15 }
    };
    static const ExpectedCall pascalExpected[] = {
        { SCE_PAS_COMMENT, 1 }, { SCE_PAS_COMMENT2, 1 }, { SCE_PAS_COMMENTLINE, 1 },
        { SCE_PAS_WORD, 2 }, { SCE_PAS_IDENTIFIER, 3 },
        { SCE_PAS_STRING, 4 }, { SCE_PAS_CHARACTER, 4 }, { SCE_PAS_STRINGEOL, 4 },
        { SCE_PAS_MULTILINESTRING, 4 },
        { SCE_PAS_NUMBER, 5 }, { SCE_PAS_HEXNUMBER, 5 },
        { SCE_PAS_OPERATOR, 6 }, { SCE_PAS_ASM, 7 },
        { SCE_PAS_PREPROCESSOR, 8 }, { SCE_PAS_PREPROCESSOR2, 8 }
    };
    static const ExpectedCall fortranExpected[] = {
        { SCE_F_IDENTIFIER, 1 }, { SCE_F_PREPROCESSOR, 2 }, { SCE_F_COMMENT, 3 },
        { SCE_F_WORD, 4 }, { SCE_F_WORD2, 5 }, { SCE_F_WORD3, 6 },
        { SCE_F_STRING1, 7 }, { SCE_F_STRING2, 8 }, { SCE_F_STRINGEOL, 9 },
        { SCE_F_NUMBER, 10 }, { SCE_F_OPERATOR, 11 }, { SCE_F_OPERATOR2, 11 },
        { SCE_F_LABEL, 12 }, { SCE_F_CONTINUATION, 13 }
    };
    static const ExpectedCall vhdlExpected[] = {
        { SCE_VHDL_COMMENT, 1 }, { SCE_VHDL_COMMENTLINEBANG, 1 }, { SCE_VHDL_BLOCK_COMMENT, 1 },
        { SCE_VHDL_NUMBER, 2 }, { SCE_VHDL_STRING, 3 }, { SCE_VHDL_STRINGEOL, 3 },
        { SCE_VHDL_OPERATOR, 4 }, { SCE_VHDL_IDENTIFIER, 5 }, { SCE_VHDL_KEYWORD, 6 },
        { SCE_VHDL_STDOPERATOR, 7 }, { SCE_VHDL_ATTRIBUTE, 8 }, { SCE_VHDL_STDFUNCTION, 9 },
        { SCE_VHDL_STDPACKAGE, 10 }, { SCE_VHDL_STDTYPE, 11 }, { SCE_VHDL_USERWORD, 12 }
    };
    static const ExpectedCall verilogExpected[] = {
        { SCE_V_COMMENT, 1 }, { SCE_V_COMMENTLINE, 1 }, { SCE_V_COMMENTLINEBANG, 1 },
        { SCE_V_COMMENT_WORD, 1 }, { SCE_V_NUMBER, 2 },
        { SCE_V_WORD, 3 }, { SCE_V_WORD2, 3 }, { SCE_V_WORD3, 4 },
        { SCE_V_STRING, 5 }, { SCE_V_STRINGEOL, 5 }, { SCE_V_PREPROCESSOR, 6 },
        { SCE_V_OPERATOR, 7 }, { SCE_V_IDENTIFIER, 8 }, { SCE_V_USER, 9 },
        { SCE_V_INPUT, 10 }, { SCE_V_OUTPUT, 11 }, { SCE_V_INOUT, 12 }, { SCE_V_PORT_CONNECT, 13 }
    };
    static const ExpectedCall cmakeExpected[] = {
        { SCE_CMAKE_COMMENT, 1 },
        { SCE_CMAKE_STRINGDQ, 2 }, { SCE_CMAKE_STRINGLQ, 2 }, { SCE_CMAKE_STRINGRQ, 2 },
        { SCE_CMAKE_COMMANDS, 3 }, { SCE_CMAKE_PARAMETERS, 4 }, { SCE_CMAKE_VARIABLE, 5 },
        { SCE_CMAKE_WHILEDEF, 6 }, { SCE_CMAKE_FOREACHDEF, 7 }, { SCE_CMAKE_IFDEFINEDEF, 8 },
        { SCE_CMAKE_MACRODEF, 9 }, { SCE_CMAKE_STRINGVAR, 10 }, { SCE_CMAKE_NUMBER, 11 }
    };
    static const ExpectedCall makeExpected[] = {
        { SCE_MAKE_COMMENT, 1 }, { SCE_MAKE_IDENTIFIER, 2 }, { SCE_MAKE_IDEOL, 2 },
        { SCE_MAKE_OPERATOR, 3 }, { SCE_MAKE_TARGET, 4 }, { SCE_MAKE_PREPROCESSOR, 5 }
    };
    static const ExpectedCall confExpected[] = {
        { SCE_CONF_COMMENT, 1 }, { SCE_CONF_STRING, 2 }, { SCE_CONF_NUMBER, 3 },
        { SCE_CONF_DIRECTIVE, 4 }, { SCE_CONF_IP, 5 }
    };
    static const ExpectedCall propsExpected[] = {
        { SCE_PROPS_KEY, 1 }, { SCE_PROPS_COMMENT, 2 }, { SCE_PROPS_SECTION, 3 },
        { SCE_PROPS_ASSIGNMENT, 4 }, { SCE_PROPS_DEFVAL, 5 }
    };
    static const ExpectedCall ahkExpected[] = {
        { SCE_AHK_COMMENTLINE, 1 }, { SCE_AHK_COMMENTBLOCK, 1 }, { SCE_AHK_COMMENTDOC, 2 },
        { SCE_AHK_ESCAPE, 3 }, { SCE_AHK_SYNOPERATOR, 4 }, { SCE_AHK_EXPOPERATOR, 5 },
        { SCE_AHK_STRING, 6 }, { SCE_AHK_NUMBER, 7 }, { SCE_AHK_IDENTIFIER, 8 },
        { SCE_AHK_VARREF, 9 }, { SCE_AHK_LABEL, 10 }, { SCE_AHK_HOTSTRINGOPT, 11 },
        { SCE_AHK_WORD_CF, 12 }, { SCE_AHK_WORD_CMD, 13 }, { SCE_AHK_WORD_FN, 14 },
        { SCE_AHK_WORD_DIR, 15 }, { SCE_AHK_WORD_KB, 16 }, { SCE_AHK_WORD_VAR, 17 },
        { SCE_AHK_WORD_SP, 18 }, { SCE_AHK_WORD_UD, 19 }, { SCE_AHK_VARREFKW, 20 }
    };
    static const ExpectedCall cssExpected[] = {
        { SCE_CSS_COMMENT, 1 }, { SCE_CSS_TAG, 2 }, { SCE_CSS_CLASS, 3 },
        { SCE_CSS_ID, 4 }, { SCE_CSS_ATTRIBUTE, 5 },
        { SCE_CSS_PSEUDOCLASS, 6 }, { SCE_CSS_EXTENDED_PSEUDOCLASS, 6 },
        { SCE_CSS_PSEUDOELEMENT, 7 }, { SCE_CSS_EXTENDED_PSEUDOELEMENT, 7 },
        { SCE_CSS_IDENTIFIER, 8 }, { SCE_CSS_IDENTIFIER2, 8 },
        { SCE_CSS_IDENTIFIER3, 8 }, { SCE_CSS_EXTENDED_IDENTIFIER, 8 },
        { SCE_CSS_DOUBLESTRING, 9 }, { SCE_CSS_SINGLESTRING, 9 },
        { SCE_CSS_VALUE, 10 }, { SCE_CSS_OPERATOR, 11 }, { SCE_CSS_IMPORTANT, 12 },
        { SCE_CSS_DIRECTIVE, 13 }, { SCE_CSS_GROUP_RULE, 14 }, { SCE_CSS_VARIABLE, 15 },
        { SCE_CSS_UNKNOWN_PSEUDOCLASS, 16 }, { SCE_CSS_UNKNOWN_IDENTIFIER, 17 }
    };
    static const ExpectedCall markdownExpected[] = {
        { SCE_MARKDOWN_LINE_BEGIN, 1 },
        { SCE_MARKDOWN_STRONG1, 2 }, { SCE_MARKDOWN_STRONG2, 2 },
        { SCE_MARKDOWN_EM1, 3 }, { SCE_MARKDOWN_EM2, 3 },
        { SCE_MARKDOWN_HEADER1, 4 }, { SCE_MARKDOWN_HEADER2, 5 }, { SCE_MARKDOWN_HEADER3, 6 },
        { SCE_MARKDOWN_HEADER4, 7 }, { SCE_MARKDOWN_HEADER5, 8 }, { SCE_MARKDOWN_HEADER6, 9 },
        { SCE_MARKDOWN_PRECHAR, 10 }, { SCE_MARKDOWN_ULIST_ITEM, 11 }, { SCE_MARKDOWN_OLIST_ITEM, 12 },
        { SCE_MARKDOWN_BLOCKQUOTE, 13 }, { SCE_MARKDOWN_STRIKEOUT, 14 }, { SCE_MARKDOWN_HRULE, 15 },
        { SCE_MARKDOWN_LINK, 16 }, { SCE_MARKDOWN_CODE, 17 }, { SCE_MARKDOWN_CODE2, 17 },
        { SCE_MARKDOWN_CODEBK, 17 }, { SCE_MARKDOWN_FRONT_MARK, 18 },
        { SCE_MARKDOWN_FRONT, 19 }, { SCE_MARKDOWN_FRONT_KEY, 20 }
    };
    static const ExpectedCall innoExpected[] = {
        { SCE_INNO_COMMENT, 1 }, { SCE_INNO_KEYWORD, 2 }, { SCE_INNO_PARAMETER, 3 },
        { SCE_INNO_SECTION, 4 }, { SCE_INNO_PREPROC, 5 }, { SCE_INNO_INLINE_EXPANSION, 6 },
        { SCE_INNO_COMMENT_PASCAL, 7 }, { SCE_INNO_KEYWORD_PASCAL, 8 },
        { SCE_INNO_STRING_DOUBLE, 9 }, { SCE_INNO_STRING_SINGLE, 9 }
    };
    static const ExpectedCall nsisExpected[] = {
        { SCE_NSIS_COMMENT, 1 }, { SCE_NSIS_COMMENTBOX, 1 },
        { SCE_NSIS_STRINGDQ, 2 }, { SCE_NSIS_STRINGLQ, 2 }, { SCE_NSIS_STRINGRQ, 2 },
        { SCE_NSIS_FUNCTION, 3 }, { SCE_NSIS_VARIABLE, 4 }, { SCE_NSIS_STRINGVAR, 5 },
        { SCE_NSIS_NUMBER, 6 }, { SCE_NSIS_LABEL, 7 }, { SCE_NSIS_SECTIONDEF, 8 },
        { SCE_NSIS_SUBSECTIONDEF, 9 }, { SCE_NSIS_SECTIONGROUP, 10 }, { SCE_NSIS_FUNCTIONDEF, 11 },
        { SCE_NSIS_PAGEEX, 12 }, { SCE_NSIS_IFDEFINEDEF, 13 }, { SCE_NSIS_MACRODEF, 14 }
    };
    static const ExpectedCall abaqusExpected[] = {
        { SCE_ABAQUS_COMMENT, 1 }, { SCE_ABAQUS_COMMENTBLOCK, 1 },
        { SCE_ABAQUS_NUMBER, 2 }, { SCE_ABAQUS_STRING, 3 }, { SCE_ABAQUS_OPERATOR, 4 },
        { SCE_ABAQUS_STARCOMMAND, 5 }, { SCE_ABAQUS_ARGUMENT, 6 }, { SCE_ABAQUS_PROCESSOR, 7 },
        { SCE_ABAQUS_COMMAND, 8 }, { SCE_ABAQUS_SLASHCOMMAND, 9 }, { SCE_ABAQUS_WORD, 10 },
        { SCE_ABAQUS_FUNCTION, 11 }
    };
    static const ExpectedCall apdlExpected[] = {
        { SCE_APDL_COMMENT, 1 }, { SCE_APDL_COMMENTBLOCK, 1 },
        { SCE_APDL_NUMBER, 2 }, { SCE_APDL_STRING, 3 }, { SCE_APDL_OPERATOR, 4 },
        { SCE_APDL_PROCESSOR, 5 }, { SCE_APDL_COMMAND, 6 }, { SCE_APDL_SLASHCOMMAND, 7 },
        { SCE_APDL_STARCOMMAND, 8 }, { SCE_APDL_ARGUMENT, 9 }, { SCE_APDL_FUNCTION, 10 },
        { SCE_APDL_WORD, 11 }
    };
    static const ExpectedCall csvExpected[] = {
        { STYLE_LINENUMBER, 1 },
        { (SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT) & 0xFF, 2 },
        { ((SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT) >> 8) & 0xFF, 2 },
        { SCE_CSV_COLUMN_0, 3 }, { SCE_CSV_COLUMN_1, 4 }, { SCE_CSV_COLUMN_2, 5 },
        { SCE_CSV_COLUMN_3, 6 }, { SCE_CSV_COLUMN_4, 7 }, { SCE_CSV_COLUMN_5, 8 },
        { SCE_CSV_COLUMN_6, 9 }, { SCE_CSV_COLUMN_7, 10 }, { SCE_CSV_COLUMN_8, 11 },
        { SCE_CSV_COLUMN_9, 12 }
    };
    static const ExpectedCall diffExpected[] = {
        { SCE_DIFF_COMMENT, 1 }, { SCE_DIFF_COMMAND, 2 }, { SCE_DIFF_HEADER, 3 },
        { SCE_DIFF_POSITION, 4 },
        { SCE_DIFF_ADDED, 5 }, { SCE_DIFF_PATCH_ADD, 5 }, { SCE_DIFF_REMOVED_PATCH_ADD, 5 },
        { SCE_DIFF_DELETED, 6 }, { SCE_DIFF_PATCH_DELETE, 6 }, { SCE_DIFF_REMOVED_PATCH_DELETE, 6 },
        { SCE_DIFF_CHANGED, 7 }
    };
    static const ExpectedCall asmExpected[] = {
        { SCE_ASM_COMMENT, 1 }, { SCE_ASM_COMMENTBLOCK, 1 }, { SCE_ASM_IDENTIFIER, 2 },
        { SCE_ASM_STRING, 3 }, { SCE_ASM_CHARACTER, 3 }, { SCE_ASM_STRINGEOL, 3 },
        { SCE_ASM_NUMBER, 4 }, { SCE_ASM_OPERATOR, 5 }, { SCE_ASM_CPUINSTRUCTION, 6 },
        { SCE_ASM_MATHINSTRUCTION, 7 }, { SCE_ASM_EXTINSTRUCTION, 8 },
        { SCE_ASM_DIRECTIVE, 9 }, { SCE_ASM_DIRECTIVEOPERAND, 10 }, { SCE_ASM_REGISTER, 11 }
    };
    static const ExpectedCall registryExpected[] = {
        { SCE_REG_COMMENT, 1 }, { SCE_REG_VALUENAME, 2 }, { SCE_REG_STRING, 3 },
        { SCE_REG_VALUETYPE, 4 }, { SCE_REG_HEXDIGIT, 5 }, { SCE_REG_ADDEDKEY, 6 },
        { SCE_REG_DELETEDKEY, 7 }, { SCE_REG_ESCAPED, 8 }, { SCE_REG_STRING_GUID, 9 },
        { SCE_REG_KEYPATH_GUID, 10 }, { SCE_REG_PARAMETER, 11 }, { SCE_REG_OPERATOR, 12 }
    };
    static const ExpectedCall au3Expected[] = {
        { SCE_AU3_COMMENT, 1 }, { SCE_AU3_COMMENTBLOCK, 1 }, { SCE_AU3_NUMBER, 2 },
        { SCE_AU3_FUNCTION, 3 }, { SCE_AU3_UDF, 4 }, { SCE_AU3_KEYWORD, 5 },
        { SCE_AU3_MACRO, 6 }, { SCE_AU3_STRING, 7 }, { SCE_AU3_OPERATOR, 8 },
        { SCE_AU3_VARIABLE, 9 }, { SCE_AU3_SENT, 10 }, { SCE_AU3_PREPROCESSOR, 11 },
        { SCE_AU3_SPECIAL, 12 }, { SCE_AU3_EXPAND, 13 }, { SCE_AU3_COMOBJ, 14 }
    };
    static const ExpectedCall kixExpected[] = {
        { SCE_KIX_KEYWORD, 1 }, { SCE_KIX_FUNCTIONS, 2 }, { SCE_KIX_MACRO, 3 },
        { SCE_KIX_COMMENT, 4 }, { SCE_KIX_COMMENTSTREAM, 4 },
        { SCE_KIX_STRING1, 5 }, { SCE_KIX_STRING2, 5 }, { SCE_KIX_OPERATOR, 6 },
        { SCE_KIX_NUMBER, 7 }, { SCE_KIX_VAR, 8 }, { SCE_KIX_IDENTIFIER, 9 }
    };
    static const ExpectedCall avsExpected[] = {
        { SCE_AVS_COMMENTLINE, 1 }, { SCE_AVS_COMMENTBLOCK, 1 }, { SCE_AVS_COMMENTBLOCKN, 1 },
        { SCE_AVS_OPERATOR, 2 }, { SCE_AVS_STRING, 3 }, { SCE_AVS_TRIPLESTRING, 3 },
        { SCE_AVS_NUMBER, 4 }, { SCE_AVS_KEYWORD, 5 }, { SCE_AVS_IDENTIFIER, 6 },
        { SCE_AVS_FILTER, 7 }, { SCE_AVS_PLUGIN, 8 }, { SCE_AVS_FUNCTION, 9 },
        { SCE_AVS_CLIPPROP, 10 }, { SCE_AVS_USERDFN, 11 }
    };
    static const ExpectedCall coffeeExpected[] = {
        { SCE_COFFEESCRIPT_COMMENT, 1 }, { SCE_COFFEESCRIPT_COMMENTLINE, 1 },
        { SCE_COFFEESCRIPT_COMMENTDOC, 1 }, { SCE_COFFEESCRIPT_COMMENTBLOCK, 1 },
        { SCE_COFFEESCRIPT_STRING, 2 }, { SCE_COFFEESCRIPT_STRINGEOL, 2 },
        { SCE_COFFEESCRIPT_STRINGRAW, 2 }, { SCE_COFFEESCRIPT_PREPROCESSOR, 3 },
        { SCE_COFFEESCRIPT_IDENTIFIER, 4 }, { SCE_COFFEESCRIPT_OPERATOR, 5 },
        { SCE_COFFEESCRIPT_NUMBER, 6 }, { SCE_COFFEESCRIPT_REGEX, 7 },
        { SCE_COFFEESCRIPT_VERBOSE_REGEX, 7 }, { SCE_COFFEESCRIPT_VERBOSE_REGEX_COMMENT, 7 },
        { SCE_COFFEESCRIPT_GLOBALCLASS, 8 }, { SCE_COFFEESCRIPT_WORD, 9 },
        { SCE_COFFEESCRIPT_WORD2, 9 }, { SCE_COFFEESCRIPT_VERBATIM, 10 },
        { SCE_COFFEESCRIPT_TRIPLEVERBATIM, 10 }, { SCE_COFFEESCRIPT_INSTANCEPROPERTY, 11 }
    };
    static const ExpectedCall nimExpected[] = {
        { SCE_NIM_COMMENT, 1 }, { SCE_NIM_COMMENTDOC, 1 }, { SCE_NIM_COMMENTLINE, 1 },
        { SCE_NIM_COMMENTLINEDOC, 1 }, { SCE_NIM_WORD, 2 }, { SCE_NIM_IDENTIFIER, 3 },
        { SCE_NIM_STRING, 4 }, { SCE_NIM_STRINGEOL, 4 }, { SCE_NIM_CHARACTER, 5 },
        { SCE_NIM_TRIPLEDOUBLE, 6 }, { SCE_NIM_TRIPLE, 7 }, { SCE_NIM_NUMBER, 8 },
        { SCE_NIM_OPERATOR, 9 }, { SCE_NIM_FUNCNAME, 10 }, { SCE_NIM_NUMERROR, 11 }
    };
    static const ExpectedCall tclExpected[] = {
        { SCE_TCL_COMMENT, 1 }, { SCE_TCL_COMMENTLINE, 1 }, { SCE_TCL_COMMENT_BOX, 1 },
        { SCE_TCL_BLOCK_COMMENT, 1 }, { SCE_TCL_WORD, 2 }, { SCE_TCL_WORD2, 2 },
        { SCE_TCL_WORD3, 2 }, { SCE_TCL_WORD_IN_QUOTE, 2 }, { SCE_TCL_NUMBER, 3 },
        { SCE_TCL_IN_QUOTE, 4 }, { SCE_TCL_OPERATOR, 5 }, { SCE_TCL_IDENTIFIER, 6 },
        { SCE_TCL_SUBSTITUTION, 7 }, { SCE_TCL_SUB_BRACE, 7 }, { SCE_TCL_MODIFIER, 8 },
        { SCE_TCL_EXPAND, 9 }, { SCE_TCL_WORD4, 10 }, { SCE_TCL_WORD5, 10 },
        { SCE_TCL_WORD6, 10 }, { SCE_TCL_WORD7, 10 }, { SCE_TCL_WORD8, 11 }
    };
    static const ExpectedCall latexExpected[] = {
        { SCE_L_COMMAND, 1 }, { SCE_L_SHORTCMD, 1 }, { SCE_L_CMDOPT, 1 },
        { SCE_L_COMMENT, 2 }, { SCE_L_COMMENT2, 2 }, { SCE_L_MATH, 3 },
        { SCE_L_MATH2, 3 }, { SCE_L_SPECIAL, 4 }, { SCE_L_TAG, 5 },
        { SCE_L_TAG2, 5 }, { SCE_L_VERBATIM, 6 }, { SCE_L_ERROR, 7 }
    };
    static const ExpectedCall sqlExpected[] = {
        { SCE_SQL_COMMENT, 1 }, { SCE_SQL_COMMENTLINE, 1 }, { SCE_SQL_COMMENTDOC, 1 },
        { SCE_SQL_COMMENTLINEDOC, 1 }, { SCE_SQL_COMMENTDOCKEYWORD, 1 },
        { SCE_SQL_COMMENTDOCKEYWORDERROR, 1 }, { SCE_SQL_WORD, 2 }, { SCE_SQL_USER1, 3 },
        { SCE_SQL_STRING, 4 }, { SCE_SQL_CHARACTER, 4 }, { SCE_SQL_IDENTIFIER, 5 },
        { SCE_SQL_QUOTEDIDENTIFIER, 6 }, { SCE_SQL_NUMBER, 7 }, { SCE_SQL_OPERATOR, 8 },
        { SCE_SQL_COMMENTDOC, 9 }, { SCE_SQL_COMMENTDOCKEYWORD, 10 }, { SCE_SQL_QOPERATOR, 11 }
    };
    static const ExpectedCall htmlWitnesses[] = {
        { SCE_H_SGML_COMMAND, 12 }, { SCE_HP_WORD, 21 }, { SCE_HPA_IDENTIFIER, 21 },
        { SCE_HPHP_COMMENT, 24 }, { SCE_HJ_TEMPLATELITERAL, 34 },
        { SCE_HJA_REGEX, 42 }, { SCE_HB_NUMBER, 50 }, { SCE_HBA_STRING, 56 }
    };
    static const ExpectedCall xmlWitnesses[] = {
        { SCE_H_SGML_COMMAND, 10 }, { SCE_HPHP_COMMENT, 11 }, { SCE_HJ_TEMPLATELITERAL, 11 },
        { SCE_HJA_REGEX, 11 }, { SCE_HB_NUMBER, 11 }, { SCE_HBA_STRING, 11 },
        { SCE_HP_WORD, 11 }, { SCE_HPA_IDENTIFIER, 11 }
    };
    static const ExpectedCall standardExpected[] = {
        { STYLE_LINENUMBER, 1 }, { STYLE_BRACELIGHT, 2 }, { STYLE_BRACEBAD, 3 },
        { STYLE_CONTROLCHAR, 4 }, { STYLE_INDENTGUIDE, 5 },
        { _STYLE_GETSTYLEID(STY_SEL_TXT), 6 }, { _STYLE_GETSTYLEID(STY_WHITESPACE), 7 },
        { _STYLE_GETSTYLEID(STY_NONPRINT), 8 }, { _STYLE_GETSTYLEID(STY_CUR_LN), 9 },
        { _STYLE_GETSTYLEID(STY_CARET), 10 }, { _STYLE_GETSTYLEID(STY_LONG_LN_MRK), 11 },
        { _STYLE_GETSTYLEID(STY_X_LN_SPACE), 12 }, { _STYLE_GETSTYLEID(STY_BOOK_MARK), 13 },
        { _STYLE_GETSTYLEID(STY_MARK_OCC), 14 }, { _STYLE_GETSTYLEID(STY_URL_HOTSPOT), 15 },
        { _STYLE_GETSTYLEID(STY_UNICODE_HOTSPOT), 16 }, { _STYLE_GETSTYLEID(STY_MULTI_EDIT), 17 },
        { _STYLE_GETSTYLEID(STY_CHGHIST_MODIFIED), 18 }, { _STYLE_GETSTYLEID(STY_CHGHIST_SAVED), 19 },
        { _STYLE_GETSTYLEID(STY_CHGHIST_REV_TO_MOD), 20 }, { _STYLE_GETSTYLEID(STY_CHGHIST_REV_TO_ORG), 21 },
        { _STYLE_GETSTYLEID(STY_IME_COLOR), 22 }
    };
    static const ExpectedCall textExpected[] = {
        { STYLE_LINENUMBER, 1 }, { STYLE_BRACELIGHT, 2 }
    };
    static const ExpectedCall ansiExpected[] = {
        { STYLE_LINENUMBER, 1 }, { STYLE_BRACELIGHT, 2 },
        { STYLE_BRACEBAD, 3 }, { STYLE_CONTROLCHAR, 4 }
    };

    if (wcscmp(lexJSON.pszDefExt, L"") != 0 ||
        !wcsstr(lexJSON5.pszDefExt, L"json; json5;") ||
        !lexJSON.pKeyWords || !lexJSON.pKeyWords->pszKeyWords[1] ||
        !strstr(lexJSON.pKeyWords->pszKeyWords[1], "@context") ||
        !lexJSON5.pKeyWords || !lexJSON5.pKeyWords->pszKeyWords[1] ||
        !strstr(lexJSON5.pKeyWords->pszKeyWords[1], "@context")) {
        fputs("Unexpected JSON extensions or JSON-LD keywords\n", stderr);
        return 1;
    }

    if (!lexAHK.pKeyWords || !lexAHK.pKeyWords->pszKeyWords[1] ||
        !strstr(lexAHK.pKeyWords->pszKeyWords[1], "MsgBox") ||
        !lexAHK2.pKeyWords || !lexAHK2.pKeyWords->pszKeyWords[1] ||
        lexAHK2.pKeyWords->pszKeyWords[1][0] != '\0' ||
        !lexAHK2.pKeyWords->pszKeyWords[2] || !strstr(lexAHK2.pKeyWords->pszKeyWords[2], "Map")) {
        fputs("Unexpected AutoHotkey keyword lists\n", stderr);
        return 1;
    }

    static const char* const apdlKeywords[] = { "/prep7", "solve", "/input", "*do", "elem", "sqrt" };
    if (!lexAPDL.pKeyWords) {
        fputs("Missing APDL keyword lists\n", stderr);
        return 1;
    }
    for (int i = 0; i < (int)COUNTOF(apdlKeywords); ++i) {
        if (!lexAPDL.pKeyWords->pszKeyWords[i] || !strstr(lexAPDL.pKeyWords->pszKeyWords[i], apdlKeywords[i])) {
            fprintf(stderr, "Unexpected APDL keyword list %d\n", i);
            return 1;
        }
    }

    static const char* const asmKeywords[] = { "adc", "fadd", "eax", "%define", "dword", "addpd" };
    for (int i = 0; i < (int)COUNTOF(asmKeywords); ++i) {
        if (!lexASM.pKeyWords || !lexASM.pKeyWords->pszKeyWords[i] ||
            !strstr(lexASM.pKeyWords->pszKeyWords[i], asmKeywords[i])) {
            fprintf(stderr, "Unexpected assembly keyword list %d\n", i);
            return 1;
        }
    }

    static const char* const au3Keywords[] = { "continuecase", "adlibregister", "@autoitexe", "{enter}", "#include", "", "_arrayadd" };
    static const char* const kixKeywords[] = { "endselect", "addkey", "scriptdir" };
    for (int i = 0; i < (int)COUNTOF(au3Keywords); ++i) {
        if (!lexAU3.pKeyWords || !lexAU3.pKeyWords->pszKeyWords[i] ||
            (i == 5 ? lexAU3.pKeyWords->pszKeyWords[i][0] != '\0' :
             !strstr(lexAU3.pKeyWords->pszKeyWords[i], au3Keywords[i]))) {
            fprintf(stderr, "Unexpected AutoIt keyword list %d\n", i);
            return 1;
        }
    }
    for (int i = 0; i < (int)COUNTOF(kixKeywords); ++i) {
        if (!lexKiX.pKeyWords || !lexKiX.pKeyWords->pszKeyWords[i] ||
            !strstr(lexKiX.pKeyWords->pszKeyWords[i], kixKeywords[i])) {
            fprintf(stderr, "Unexpected KiXtart keyword list %d\n", i);
            return 1;
        }
    }

    static const char* const avsKeywords[] = { "current_frame", "addborders", "ablur", "arrayadd", "framecount", "_iqa_downsample" };
    for (int i = 0; i < (int)COUNTOF(avsKeywords); ++i) {
        if (!lexAVS.pKeyWords || !lexAVS.pKeyWords->pszKeyWords[i] ||
            !strstr(lexAVS.pKeyWords->pszKeyWords[i], avsKeywords[i])) {
            fprintf(stderr, "Unexpected AviSynth keyword list %d\n", i);
            return 1;
        }
    }

    static const char* const tclKeywords[] = { "auto_execok", "bindtags", "itk_component" };
    for (int i = 0; i < (int)COUNTOF(tclKeywords); ++i) {
        if (!lexTCL.pKeyWords || !lexTCL.pKeyWords->pszKeyWords[i] ||
            !strstr(lexTCL.pKeyWords->pszKeyWords[i], tclKeywords[i])) {
            fprintf(stderr, "Unexpected Tcl keyword list %d\n", i);
            return 1;
        }
    }

    static const char* const sqlKeywords[] = { "select", "", "deprecated", "date_add", "bigint", "geometry", "avrostorage", "parquet" };
    for (int i = 0; i < (int)COUNTOF(sqlKeywords); ++i) {
        if (!lexSQL.pKeyWords || !lexSQL.pKeyWords->pszKeyWords[i] ||
            (i == 1 ? lexSQL.pKeyWords->pszKeyWords[i][0] != '\0' :
             !strstr(lexSQL.pKeyWords->pszKeyWords[i], sqlKeywords[i]))) {
            fprintf(stderr, "Unexpected SQL keyword list %d\n", i);
            return 1;
        }
    }

    static const char* const htmlKeywords[] = { "!doctype", "function", "function", "endclass", "__callstatic", "DOCTYPE" };
    for (int i = 0; i < (int)COUNTOF(htmlKeywords); ++i) {
        if (!lexHTML.pKeyWords || !lexHTML.pKeyWords->pszKeyWords[i] ||
            !strstr(lexHTML.pKeyWords->pszKeyWords[i], htmlKeywords[i])) {
            fprintf(stderr, "Unexpected HTML keyword list %d\n", i);
            return 1;
        }
    }
    if (strcmp(lexHTML.pKeyWords->pszKeyWords[1], lexJS.pKeyWords->pszKeyWords[0]) != 0) {
        fputs("HTML and JavaScript keywords differ\n", stderr);
        return 1;
    }

    if (lexStandard.pKeyWords != lexStandard2nd.pKeyWords || lexStandard.pKeyWords != lexTEXT.pKeyWords ||
        lexStandard.pKeyWords != lexANSI.pKeyWords || wcscmp(lexStandard.Styles[0].pszDefault, L"font:$Code0") != 0 ||
        wcscmp(lexStandard2nd.Styles[0].pszDefault, L"font:$Code1") != 0 ||
        wcscmp(lexTEXT.pszDefExt, L"txt; text; tmp; log; asc; doc; scp; wtx") != 0 ||
        wcscmp(lexANSI.pszDefExt, L"ans; asc; diz; nfo; FILE_ID.DIZ") != 0) {
        fputs("Unexpected standard lexer defaults\n", stderr);
        return 1;
    }
    for (int i = STY_SEL_TXT; i <= STY_IME_COLOR; ++i) {
        if (lexStandard.Styles[i].iStyle != _STYLE_GETSTYLEID(i) ||
            lexStandard2nd.Styles[i].iStyle != _STYLE_GETSTYLEID(i)) {
            fprintf(stderr, "Unexpected standard style index %d\n", i);
            return 1;
        }
    }

    if (CheckLexer(&lexCPP, SCLEX_CPP, "cpp", L"C/C++ Source Code", "alignas", 16, cppExpected, (int)COUNTOF(cppExpected)) ||
        CheckLexer(&lexJAVA, SCLEX_CPP, "cpp", L"Java Source Code", "abstract", 12, javaExpected, (int)COUNTOF(javaExpected)) ||
        CheckLexer(&lexJS, SCLEX_CPP, "cpp", L"JavaScript", "function", 12, jsExpected, (int)COUNTOF(jsExpected)) ||
        CheckLexer(&lexCS, SCLEX_CPP, "cpp", L"C# Source Code", "abstract", 15, cppExpected, (int)COUNTOF(cppExpected) - 1) ||
        CheckLexer(&lexRC, SCLEX_CPP, "cpp", L"Resource Script", "ACCELERATORS", 12, rcExpected, (int)COUNTOF(rcExpected)) ||
        CheckLexer(&lexD, SCLEX_D, "d", L"D Source Code", "abstract", 10, dExpected, (int)COUNTOF(dExpected)) ||
        CheckLexer(&lexGo, SCLEX_D, "d", L"Go Source Code", "defer", 9, goExpected, (int)COUNTOF(goExpected)) ||
        CheckLexer(&lexPY, SCLEX_PYTHON, "python", L"Python Script", "nonlocal", 15, pythonExpected, (int)COUNTOF(pythonExpected)) ||
        CheckLexer(&lexAwk, SCLEX_PYTHON, "python", L"Awk Script", "BEGINFILE", 8, awkExpected, (int)COUNTOF(awkExpected)) ||
        CheckLexer(&lexJSON, SCLEX_JSON, "json", L"JSON", "false", 13, jsonExpected, (int)COUNTOF(jsonExpected)) ||
        CheckLexer(&lexJSON5, SCLEX_JSON5, "json5", L"JSON5", "Infinity", 13, json5Expected, (int)COUNTOF(json5Expected)) ||
        CheckLexer(&lexYAML, SCLEX_YAML, "yaml", L"YAML", "false", 10, yamlExpected, (int)COUNTOF(yamlExpected)) ||
        CheckLexer(&lexTOML, SCLEX_TOML, "toml", L"TOML Config", "+inf", 14, tomlExpected, (int)COUNTOF(tomlExpected)) ||
        CheckLexer(&lexVB, SCLEX_VB, "vb", L"Visual Basic", "function", 8, vbExpected, (int)COUNTOF(vbExpected)) ||
        CheckLexer(&lexVBS, SCLEX_VBSCRIPT, "vbscript", L"VBScript", "function", 7, vbsExpected, (int)COUNTOF(vbsExpected)) ||
        CheckLexer(&lexBASH, SCLEX_BASH, "bash", L"Shell Script", "function", 14, bashExpected, (int)COUNTOF(bashExpected)) ||
        CheckLexer(&lexBAT, SCLEX_BATCH, "batch", L"Batch Files", "setlocal", 9, batExpected, (int)COUNTOF(batExpected)) ||
        CheckLexer(&lexPS, SCLEX_POWERSHELL, "powershell", L"PowerShell Script", "function", 11, psExpected, (int)COUNTOF(psExpected)) ||
        CheckLexer(&lexRUBY, SCLEX_RUBY, "ruby", L"Ruby Script", "class", 25, rubyExpected, (int)COUNTOF(rubyExpected)) ||
        CheckLexer(&lexPL, SCLEX_PERL, "perl", L"Perl Script", "package", 34, perlExpected, (int)COUNTOF(perlExpected)) ||
        CheckLexer(&lexLUA, SCLEX_LUA, "lua", L"Lua Script", "function", 13, luaExpected, (int)COUNTOF(luaExpected)) ||
        CheckLexer(&lexR, SCLEX_R, "r", L"R-S-SPlus Statistics Code", "function", 14, rExpected, (int)COUNTOF(rExpected)) ||
        CheckLexer(&lexMATLAB, SCLEX_MATLAB, "matlab", L"MATLAB", "function", 8, matlabExpected, (int)COUNTOF(matlabExpected)) ||
        CheckLexer(&lexRust, SCLEX_RUST, "rust", L"Rust Source Code", "unsafe", 14, rustExpected, (int)COUNTOF(rustExpected)) ||
        CheckLexer(&lexKotlin, SCLEX_KOTLIN, "Kotlin", L"Kotlin Source Code", "fun", 20, kotlinExpected, (int)COUNTOF(kotlinExpected)) ||
        CheckLexer(&lexDart, SCLEX_DART, "dart", L"Dart Source Code", "async", 15, dartExpected, (int)COUNTOF(dartExpected)) ||
        CheckLexer(&lexJulia, SCLEX_JULIA, "julia", L"Julia Script", "function", 16, juliaExpected, (int)COUNTOF(juliaExpected)) ||
        CheckLexer(&lexPAS, SCLEX_PASCAL, "pascal", L"Pascal Source Code", "procedure", 9, pascalExpected, (int)COUNTOF(pascalExpected)) ||
        CheckLexer(&lexFortran, SCLEX_FORTRAN, "fortran", L"Fortran Source Code", "subroutine", 14, fortranExpected, (int)COUNTOF(fortranExpected)) ||
        CheckLexer(&lexVHDL, SCLEX_VHDL, "vhdl", L"VHDL", "architecture", 13, vhdlExpected, (int)COUNTOF(vhdlExpected)) ||
        CheckLexer(&lexVerilog, SCLEX_VERILOG, "verilog", L"Verilog HDL", "module", 14, verilogExpected, (int)COUNTOF(verilogExpected)) ||
        CheckLexer(&lexSysVerilog, SCLEX_SYSVERILOG, "sysverilog", L"SystemVerilog HDVL", "always_comb", 14, verilogExpected, (int)COUNTOF(verilogExpected)) ||
        CheckLexer(&lexCmake, SCLEX_CMAKE, "cmake", L"Cmake Script", "add_library", 12, cmakeExpected, (int)COUNTOF(cmakeExpected)) ||
        CheckLexer(&lexMAK, SCLEX_MAKEFILE, "makefile", L"Makefiles", NULL, 6, makeExpected, (int)COUNTOF(makeExpected)) ||
        CheckLexer(&lexCONF, SCLEX_CONF, "conf", L"Apache Config Files", "documentroot", 6, confExpected, (int)COUNTOF(confExpected)) ||
        CheckLexer(&lexPROPS, SCLEX_PROPERTIES, "props", L"Configuration Files", NULL, 6, propsExpected, (int)COUNTOF(propsExpected)) ||
        CheckLexer(&lexAHK, SCLEX_AHK, "ahk", L"AutoHotkey Script", "Break", 21, ahkExpected, (int)COUNTOF(ahkExpected)) ||
        CheckLexer(&lexAHK2, SCLEX_AHK2, "ahk2", L"AutoHotkey v2 Script", "Break", 21, ahkExpected, (int)COUNTOF(ahkExpected)) ||
        CheckLexer(&lexCSS, SCLEX_CSS, "css", L"CSS Style Sheets", "background", 18, cssExpected, (int)COUNTOF(cssExpected)) ||
        CheckLexer(&lexMARKDOWN, SCLEX_MARKDOWN, "markdown", L"Markdown", NULL, 21, markdownExpected, (int)COUNTOF(markdownExpected)) ||
        CheckLexer(&lexINNO, SCLEX_INNOSETUP, "inno", L"Inno Setup Script", "setup", 10, innoExpected, (int)COUNTOF(innoExpected)) ||
        CheckLexer(&lexNSIS, SCLEX_NSIS, "nsis", L"NSIS Script", "!include", 15, nsisExpected, (int)COUNTOF(nsisExpected)) ||
        CheckLexer(&lexAbaqus, SCLEX_ABAQUS, "abaqus", L"ABAQUS", NULL, 12, abaqusExpected, (int)COUNTOF(abaqusExpected)) ||
        CheckLexer(&lexAPDL, SCLEX_APDL, "apdl", L"ANSYS APDL", "/prep7", 12, apdlExpected, (int)COUNTOF(apdlExpected)) ||
        CheckLexer(&lexCSV, SCLEX_CSV, "CSV", L"CSV Prism", NULL, 13, csvExpected, (int)COUNTOF(csvExpected)) ||
        CheckLexer(&lexDIFF, SCLEX_DIFF, "diff", L"Diff Files", NULL, 8, diffExpected, (int)COUNTOF(diffExpected)) ||
        CheckLexer(&lexASM, SCLEX_ASM, "asm", L"Assembly Script", "adc", 12, asmExpected, (int)COUNTOF(asmExpected)) ||
        CheckLexer(&lexRegistry, SCLEX_REGISTRY, "registry", L"Registry Files", NULL, 13, registryExpected, (int)COUNTOF(registryExpected)) ||
        CheckLexer(&lexAU3, SCLEX_AU3, "au3", L"AutoIt3 Script", "continuecase", 15, au3Expected, (int)COUNTOF(au3Expected)) ||
        CheckLexer(&lexKiX, SCLEX_KIX, "kix", L"KiXtart Script", "endselect", 10, kixExpected, (int)COUNTOF(kixExpected)) ||
        CheckLexer(&lexAVS, SCLEX_AVS, "avs", L"AviSynth Script", "current_frame", 12, avsExpected, (int)COUNTOF(avsExpected)) ||
        CheckLexer(&lexCOFFEESCRIPT, SCLEX_COFFEESCRIPT, "coffeescript", L"Coffeescript", NULL, 12, coffeeExpected, (int)COUNTOF(coffeeExpected)) ||
        CheckLexer(&lexNim, SCLEX_NIM, "nim", L"Nim Source Code", "converter", 12, nimExpected, (int)COUNTOF(nimExpected)) ||
        CheckLexer(&lexTCL, SCLEX_TCL, "tcl", L"Tcl Script", "auto_execok", 12, tclExpected, (int)COUNTOF(tclExpected)) ||
        CheckLexer(&lexLATEX, SCLEX_LATEX, "latex", L"LaTeX Files", NULL, 8, latexExpected, (int)COUNTOF(latexExpected)) ||
        CheckLexer(&lexSQL, SCLEX_SQL, "sql", L"SQL Query", "select", 12, sqlExpected, (int)COUNTOF(sqlExpected)) ||
        CheckWebLexer(&lexHTML, SCLEX_HTML, "hypertext", L"Web Source Code", "!doctype", 62, htmlWitnesses, (int)COUNTOF(htmlWitnesses), 167, 0x172621bau) ||
        CheckWebLexer(&lexXML, SCLEX_XML, "xml", L"XML Document", NULL, 12, xmlWitnesses, (int)COUNTOF(xmlWitnesses), 138, 0x154c1dd9u) ||
        CheckLexer(&lexStandard, SCLEX_NULL, "null", L"Common Base", NULL, 23, standardExpected, (int)COUNTOF(standardExpected)) ||
        CheckLexer(&lexStandard2nd, SCLEX_NULL, "null", L"2nd Common Base", NULL, 23, standardExpected, (int)COUNTOF(standardExpected)) ||
        CheckLexer(&lexTEXT, SCLEX_NULL, "null", L"Text Files", NULL, 3, textExpected, (int)COUNTOF(textExpected)) ||
        CheckLexer(&lexANSI, SCLEX_NULL, "null", L"ANSI Art", NULL, 5, ansiExpected, (int)COUNTOF(ansiExpected)) ||
        CheckDefaultFontSize(&lexStandard, GLOBAL_INITIAL_FONTSIZE) ||
        CheckDefaultFontSize(&lexStandard2nd, GLOBAL_INITIAL_FONTSIZE) ||
        CheckDefaultFontSize(&lexTEXT, 10.0f) ||
        CheckDefaultFontSize(&lexANSI, 10.0f)) {
        return 1;
    }
    return 0;
}
