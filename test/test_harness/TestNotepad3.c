#include "TestNotepad3Support.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define CHECK(expr) do { if (!(expr)) { printf("Failed at line %d: %s\n", __LINE__, #expr); return 1; } } while (0)

int te_supports_64bit(void) { return 0; }

static int CheckFormat(int mode, double value, bool logical, const char* expected, const wchar_t* expectedW)
{
    char text[128];
    wchar_t textW[128];
    TestSetTinyExprMode(mode);
    TinyExprToStringA(text, sizeof(text), value, logical);
    TinyExprToString(textW, _countof(textW), value, logical);
    CHECK(strcmp(text, expected) == 0);
    CHECK(wcscmp(textW, expectedW) == 0);
    return 0;
}

int main(void)
{
    CHECK(!CheckFormat(0, 123.0, false, "123", L"123"));
    CHECK(!CheckFormat(0, 1.25, false, "1.25", L"1.25"));
    CHECK(!CheckFormat(1, 0.0, true, "false", L"false"));
    CHECK(!CheckFormat(2, 1.0, true, "true", L"true"));
    CHECK(!CheckFormat(1, 2.0, true, "0x2", L"0x2"));
    CHECK(!CheckFormat(1, -1.0, false, "0xFFFFFFFF", L"0xFFFFFFFF"));
    CHECK(!CheckFormat(2, 0.0, false, "0b0", L"0b0"));
    CHECK(!CheckFormat(2, 5.0, false, "0b101", L"0b101"));
    CHECK(!CheckFormat(2, 2.5, false, "0b11", L"0b11"));
    CHECK(!CheckFormat(1, 2147483648.0, false, "2147483648", L"2147483648"));

    char shortText[4] = "xxx";
    wchar_t shortTextW[4] = L"xxx";
    TestSetTinyExprMode(2);
    TinyExprToStringA(shortText, sizeof(shortText), 5.0, false);
    TinyExprToString(shortTextW, _countof(shortTextW), 5.0, false);
    CHECK(strcmp(shortText, "0b1") == 0 && wcscmp(shortTextW, L"0b1") == 0);
    puts("Notepad3 formatting checks passed");
    return 0;
}
