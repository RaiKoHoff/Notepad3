#include "TestMRUIconSupport.h"
#include <stdio.h>
#include <wchar.h>

SETTINGS_T Settings;

static int failures;
static int calls;
static UINT seenFlags[2];
static WCHAR seenPaths[2][MAX_PATH];
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

DWORD_PTR WINAPI TestMRUGetFileInfo(LPCWSTR path, DWORD attributes, SHFILEINFOW* info, UINT size, UINT flags)
{
    (void)attributes;
    (void)size;
    if (calls < 2) {
        seenFlags[calls] = flags;
        StringCchCopyW(seenPaths[calls], ARRAYSIZE(seenPaths[calls]), path);
    }
    ++calls;
    info->iIcon = 0;
    return 1;
}

int main(void)
{
    WCHAR executable[MAX_PATH];
    CHECK(GetModuleFileNameW(NULL, executable, ARRAYSIZE(executable)) > 0);
    WCHAR missing[MAX_PATH];
    CHECK(SUCCEEDED(StringCchPrintfW(missing, ARRAYSIZE(missing), L"%s.missing", executable)));
    HPATHL absent = Path_Allocate(missing);
    HPATHL present = Path_Allocate(executable);
    CHECK(absent && present);
    if (absent && present) {
        LVITEMW first = { 0 }, second = { 0 };
        FileMRULookupIcon(absent, true, &first);
        FileMRULookupIcon(present, true, &second);
        CHECK(calls == 2);
        CHECK((seenFlags[0] & SHGFI_USEFILEATTRIBUTES) && !(seenFlags[0] & SHGFI_ATTRIBUTES));
        CHECK(!(seenFlags[1] & SHGFI_USEFILEATTRIBUTES) && (seenFlags[1] & SHGFI_ATTRIBUTES));
        CHECK(wcscmp(seenPaths[1], executable) == 0);
        CHECK((first.mask & LVIF_IMAGE) && (second.mask & LVIF_IMAGE));
    }
    Path_Release(absent);
    Path_Release(present);
    if (failures) return 1;
    puts("MRU icon worker tests passed.");
    return 0;
}
