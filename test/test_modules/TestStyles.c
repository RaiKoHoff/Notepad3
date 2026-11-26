#include <stdio.h>
#include <wchar.h>
#include "Helpers.h"
#include "Styles.h"

PEDITLEXER Style_SniffShebang(char* text);
PEDITLEXER Style_MatchLexer(LPCWSTR match, bool checkNames);
PEDITLEXER Style_WildcardMatchLexer(LPCWSTR fileName);
LPWSTR Style_TestCopy(LPCWSTR input);
void Style_TestFree(LPWSTR* value);

static int failures;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

int main(void)
{
    char python[] = "#!/usr/bin/env -S python3.11 -u\n";
    char shell[] = "#! C:\\bin\\bash-5.2\n";
    char unknown[] = "#!/usr/bin/unknown\n";
    char noShebang[] = "python\n";
    CHECK(Style_SniffShebang(python) == &lexPY);
    CHECK(Style_SniffShebang(shell) == &lexBASH);
    CHECK(Style_SniffShebang(unknown) == NULL);
    CHECK(Style_SniffShebang(noShebang) == NULL);

    wcscpy_s(lexCPP.szExtensions, ARRAYSIZE(lexCPP.szExtensions), L"cpp; c; *.hpp; Makefile");
    wcscpy_s(lexPY.szExtensions, ARRAYSIZE(lexPY.szExtensions), L"py; *.pyw; .pythonrc; \\legacy");
    CHECK(Style_MatchLexer(L"cpp", false) == &lexCPP);
    CHECK(Style_MatchLexer(L"CPP", false) == &lexCPP);
    CHECK(Style_MatchLexer(L"cp", false) == NULL);
    CHECK(Style_MatchLexer(L"python-mode", true) == &lexPY);
    CHECK(Style_MatchLexer(L"x", true) == NULL);
    CHECK(Style_WildcardMatchLexer(L"HEADER.HPP") == &lexCPP);
    CHECK(Style_WildcardMatchLexer(L"Makefile") == NULL);
    CHECK(Style_WildcardMatchLexer(L".pythonrc") == &lexPY);
    CHECK(Style_WildcardMatchLexer(L"legacy") == NULL);
    CHECK(Style_WildcardMatchLexer(L"unknown.zzz") == NULL);
    CHECK(Style_WildcardMatchLexer(L"") == NULL);

    WCHAR original[] = L"style";
    LPWSTR copy = Style_TestCopy(original);
    CHECK(copy && copy != original && wcscmp(copy, original) == 0);
    original[0] = L'S';
    CHECK(copy && wcscmp(copy, L"style") == 0);
    Style_TestFree(&copy);
    copy = Style_TestCopy(NULL);
    CHECK(copy == NULL);
    Style_TestFree(&copy);

    if (failures) return 1;
    puts("Styles C-caller tests passed.");
    return 0;
}
