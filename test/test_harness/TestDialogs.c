#include "TestDialogsSupport.h"
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

int TestDefaultButton(UINT type);
int TestFilterSpec(LPCWSTR filter, COMDLG_FILTERSPEC** spec);

static bool failAlloc;
void* TestDialogAlloc(size_t size, DWORD flags)
{
    (void)flags;
    return failAlloc ? NULL : calloc(1, size);
}

#define CHECK(expr) do { if (!(expr)) { printf("Failed at line %d: %s\n", __LINE__, #expr); return 1; } } while (0)

int main(void)
{
    const struct { UINT type; int expected; } buttons[] = {
        { MB_OK, IDOK }, { MB_OK | MB_DEFBUTTON2, IDOK },
        { MB_YESNO, IDYES }, { MB_YESNO | MB_DEFBUTTON2, IDNO },
        { MB_OKCANCEL, IDOK }, { MB_OKCANCEL | MB_DEFBUTTON2, IDCANCEL },
        { MB_YESNOCANCEL, IDOK }, { MB_YESNOCANCEL | MB_DEFBUTTON2, IDNO },
        { MB_YESNOCANCEL | MB_DEFBUTTON3, IDCANCEL },
        { MB_RETRYCANCEL, IDRETRY }, { MB_RETRYCANCEL | MB_DEFBUTTON2, IDCANCEL },
        { MB_ABORTRETRYIGNORE, IDABORT }, { MB_ABORTRETRYIGNORE | MB_DEFBUTTON2, IDRETRY },
        { MB_ABORTRETRYIGNORE | MB_DEFBUTTON3, IDIGNORE },
        { MB_CANCELTRYCONTINUE, IDCANCEL }, { MB_CANCELTRYCONTINUE | MB_DEFBUTTON2, IDRETRY },
        { MB_CANCELTRYCONTINUE | MB_DEFBUTTON3, IDCONTINUE },
        { MB_FILECHANGEDNOTIFY, 0 }
    };
    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
        CHECK(TestDefaultButton(buttons[i].type | MB_ICONWARNING) == buttons[i].expected);
    }

    COMDLG_FILTERSPEC* spec = (COMDLG_FILTERSPEC*)1;
    CHECK(TestFilterSpec(NULL, &spec) == 0 && spec == NULL);
    CHECK(TestFilterSpec(L"", &spec) == 0 && spec == NULL);
    CHECK(TestFilterSpec(L"Name\0\0", &spec) == 0 && spec == NULL);
    const WCHAR filter[] = L"Text\0*.txt\0All\0*.*\0\0";
    CHECK(TestFilterSpec(filter, &spec) == 2 && spec != NULL);
    CHECK(spec[0].pszName == filter && wcscmp(spec[0].pszSpec, L"*.txt") == 0);
    CHECK(wcscmp(spec[1].pszName, L"All") == 0 && wcscmp(spec[1].pszSpec, L"*.*") == 0);
    CHECK(spec[1].pszName == filter + 11);
    free(spec);
    const WCHAR partial[] = L"Text\0*.txt\0Orphan\0\0";
    CHECK(TestFilterSpec(partial, &spec) == 1 && spec != NULL);
    CHECK(wcscmp(spec[0].pszSpec, L"*.txt") == 0);
    free(spec);
    failAlloc = true;
    spec = (COMDLG_FILTERSPEC*)1;
    CHECK(TestFilterSpec(filter, &spec) == 0 && spec == NULL);
    puts("Dialog helper checks passed");
    return 0;
}
