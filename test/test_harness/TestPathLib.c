#include <stdio.h>
#include <wchar.h>
#include <shlobj.h>
#include "PathLib.h"

SETTINGS_T Settings = { 0 };
bool TestKnownFolderCpp(HPATHL path);

static int failures;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

int main(void)
{
    HPATHL path = Path_Allocate(L"C:\\work\\folder");
    HPATHL other = Path_Allocate(L"c:\\WORK\\folder");
    HPATHL empty = Path_Allocate(L"");
    CHECK(path && other && empty);
    if (!path || !other || !empty) return 1;

    CHECK(Path_StrgComparePath(path, other, NULL, false) == 0);
    CHECK(Path_StrgComparePath(empty, empty, NULL, false) == -1);
    CHECK(Path_StrgComparePath(path, empty, NULL, false) == 1);
    CHECK(Path_Append(path, L"..\\file name.txt"));
    CHECK(wcscmp(Path_Get(path), L"C:\\work\\file name.txt") == 0);
    CHECK(wcscmp(Path_FindFileName(path), L"file name.txt") == 0);
    CHECK(wcscmp(Path_FindExtension(path), L".txt") == 0);
    CHECK(Path_QuoteSpaces(path, false));
    CHECK(wcscmp(Path_Get(path), L"\"C:\\work\\file name.txt\"") == 0);
    Path_UnQuoteSpaces(path);
    CHECK(wcscmp(Path_Get(path), L"C:\\work\\file name.txt") == 0);

    CHECK(Path_Reset(path, L"..\\child\\..\\name") > 0);
    CHECK(Path_IsRelative(path));
    CHECK(Path_Canonicalize(path));
    CHECK(wcscmp(Path_Get(path), L"..\\name") == 0);
    CHECK(Path_Reset(path, L"\\\\server\\share\\folder") > 0);
    CHECK(Path_IsUNC(path) && Path_IsValidUNC(path));
    CHECK(!Path_IsRelative(path));
    CHECK(Path_Reset(path, L"C:\\") > 0 && Path_IsRoot(path));
    CHECK(Path_Reset(path, L"folder/no-space") > 0);
    CHECK(Path_FixBackslashes(path));
    CHECK(wcscmp(Path_Get(path), L"folder\\no-space") == 0);
    CHECK(!Path_QuoteSpaces(path, false));

    WCHAR temp[MAX_PATH];
    WCHAR base[MAX_PATH];
    WCHAR expected[MAX_PATH];
    DWORD tempLength = GetTempPathW(ARRAYSIZE(temp), temp);
    CHECK(tempLength > 0 && tempLength < ARRAYSIZE(temp));
    if (!tempLength || tempLength >= ARRAYSIZE(temp)) return 1;
    CHECK(SUCCEEDED(StringCchPrintfW(base, ARRAYSIZE(base), L"%snp3-pathlib-%lu-%llu", temp,
        GetCurrentProcessId(), GetTickCount64())));
    CHECK(SUCCEEDED(StringCchPrintfW(expected, ARRAYSIZE(expected), L"%s\\two.txt", base)));
    CHECK(Path_Reset(path, L"one\\..\\two.txt") > 0);
    CHECK(Path_Reset(other, base) > 0);
    CHECK(Path_CanonicalizeEx(path, other));
    CHECK(wcscmp(Path_Get(path), expected) == 0);
    CHECK(Path_Reset(path, L"one\\..\\two.txt") > 0);
    CHECK(Path_NormalizeEx(path, other, false, false) > 0);
    CHECK(wcscmp(Path_Get(path), expected) == 0);

    WCHAR longPath[320] = L"C:\\";
    for (size_t i = 3; i < 300; ++i) longPath[i] = L'a';
    longPath[300] = L'\0';
    CHECK(Path_Reset(path, longPath) > 0);
    CHECK(Path_Canonicalize(path));
    LPCWSTR result = Path_Get(path);
    CHECK(result && (wcscmp(result, longPath) == 0 ||
        (wcsncmp(result, L"\\\\?\\", 4) == 0 && wcscmp(result + 4, longPath) == 0)));

    CHECK(Path_GetKnownFolder(&FOLDERID_Profile, other));
    CHECK(!Path_IsEmpty(other));
    CHECK(TestKnownFolderCpp(path));
    CHECK(wcscmp(Path_Get(path), Path_Get(other)) == 0);

    Path_Release(empty);
    Path_Release(other);
    Path_Release(path);
    if (failures) return 1;
    puts("PathLib C-caller tests passed.");
    return 0;
}
