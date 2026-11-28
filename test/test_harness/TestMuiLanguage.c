#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "Helpers.h"
#include "PathLib.h"
#include "Dialogs.h"
#include "Encoding.h"
#include "Config\Config.h"
#include "MuiLanguage.h"

GLOBALS_T Globals;
SETTINGS_T Settings;
SETTINGS2_T Settings2;
PATHS_T Paths;
CONSTANTS_T const Constants = { 0 };
WCHAR Default_PreferredLanguageLocaleName[] = L"en-US";
prefix_t g_mxSBPrefix[STATUS_SECTOR_COUNT];
prefix_t g_mxSBPostfix[STATUS_SECTOR_COUNT];
static int iniSets;
static int iniDeletes;

#define UNEXPECTED() do { fprintf(stderr, "Unexpected application stub: %s\n", __func__); abort(); } while (0)
WCHAR* StrNextTokW(WCHAR* text, const WCHAR* tokens) { return wcspbrk(text, tokens); }
int ReadStrgsFromCSV(LPCWSTR text, prefix_t matrix[], int count, int len, LPCWSTR def)
{ (void)text; (void)matrix; (void)count; (void)len; (void)def; UNEXPECTED(); }
void CloseNonModalDialogs(void) { UNEXPECTED(); }
void CloseApplication(void) { UNEXPECTED(); }
HPATHL PTHAPI Path_Allocate(LPCWSTR path) { (void)path; UNEXPECTED(); }
void PTHAPI Path_Release(HPATHL path) { (void)path; UNEXPECTED(); }
bool PTHAPI Path_IsExistingFile(const HPATHL path) { (void)path; UNEXPECTED(); }
void PTHAPI Path_AbsoluteFromApp(HPATHL path, bool expand) { (void)path; (void)expand; UNEXPECTED(); }
LONG InfoBoxLng(UINT type, LPCWSTR setting, UINT id, ...) { (void)type; (void)setting; (void)id; UNEXPECTED(); }
DWORD InfoBoxLastError(LPCWSTR message, DWORD error) { (void)message; (void)error; UNEXPECTED(); }
void Encoding_SetLabel(cpi_enc_t id) { (void)id; UNEXPECTED(); }
bool Has_UTF16_BOM(const char* buffer, size_t count)
{
    const unsigned char* bytes = (const unsigned char*)buffer;
    return count >= 2 && ((bytes[0] == 0xff && bytes[1] == 0xfe) || (bytes[0] == 0xfe && bytes[1] == 0xff));
}
cpi_enc_t Encoding_CountOf(void) { UNEXPECTED(); }
size_t IniFileGetString(const HPATHL path, LPCWSTR section, LPCWSTR key, LPCWSTR def, LPWSTR result, size_t count)
{ (void)path; (void)section; (void)key; (void)def; (void)result; (void)count; UNEXPECTED(); }
bool IniFileSetString(const HPATHL path, LPCWSTR section, LPCWSTR key, LPCWSTR value)
{ (void)path; (void)section; (void)key; (void)value; ++iniSets; return true; }
long IniFileGetLong(const HPATHL path, LPCWSTR section, LPCWSTR key, long def)
{ (void)path; (void)section; (void)key; (void)def; UNEXPECTED(); }
bool IniFileDelete(const HPATHL path, LPCWSTR section, LPCWSTR key, bool removeEmpty)
{ (void)path; (void)section; (void)key; (void)removeEmpty; ++iniDeletes; return true; }

static int failures;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

int main(void)
{
    WCHAR wide[128] = L"";
    char narrow[128] = "";
    unsigned const german = GetMUILanguageIndexByLocaleName(L"de-DE");

    Globals.hndlProcessHeap = GetProcessHeap();
    Globals.hInstance = GetModuleHandleW(NULL);
    Globals.hLngResContainer = GetModuleHandleW(L"kernel32.dll");

    CHECK(MuiLanguages_CountOf() == 26);
    CHECK(grepWinLang_CountOf() == MuiLanguages_CountOf());
    CHECK(grepWinLangPortApps_CountOf() == MuiLanguages_CountOf());
    CHECK(german != 0 && german < MuiLanguages_CountOf());
    CHECK(GetMUILanguageIndexByLocaleName(L"DE-de") == german);
    CHECK(GetMUILanguageIndexByLocaleName(L"not-a-real-locale") == 0);
    CHECK(wcscmp(MUI_LanguageDLLs[german].LocaleName, L"de-DE") == 0);
    CHECK(GetLangIdByLocaleName(L"en-US") == 1033);
    CHECK(GetLangIdByLocaleName(L"") == 1033);
    CHECK(GetLangIdByLocaleName(NULL) == 1033);
    CHECK(wcscmp(grepWinLangFileName[german].filename, L"German.lang") == 0);
    CHECK(wcscmp(grepWinLangFileNamePortableApps[GetMUILanguageIndexByLocaleName(L"pt-BR")].filename,
                 L"Portuguese_Brazilian.lang") == 0);
    CHECK(LoadLngStringW(40100, wide, COUNTOF(wide)) == 8);
    CHECK(wcscmp(wide, L"fallback") == 0);
    CHECK(LoadLngStringA(40100, narrow, COUNTOF(narrow)) == 8);
    CHECK(strcmp(narrow, "fallback") == 0);
    CHECK(LoadLngStringW2MB(40100, narrow, COUNTOF(narrow)) == 9);
    CHECK(strcmp(narrow, "fallback") == 0);
    CHECK(FormatLngStringW(wide, COUNTOF(wide), 40101, L"word", 42) == 7);
    CHECK(wcscmp(wide, L"word 42") == 0);
    CHECK(FormatLngStringA(narrow, COUNTOF(narrow), 40101, "word", 42) == 7);
    CHECK(strcmp(narrow, "word 42") == 0);

    Globals.bCanSaveIniFile = true;
    Globals.bIniFileFromScratch = true;
    SetMuiLanguage(german);
    CHECK(wcscmp(Globals.CurrentLngLocaleName, L"de-DE") == 0);
    CHECK(wcscmp(Settings2.PreferredLanguageLocaleName, L"de-DE") == 0);
    CHECK(iniSets == 0 && iniDeletes == 0);
    Globals.bIniFileFromScratch = false;
    SetMuiLanguage(german);
    CHECK(iniSets == 0 && iniDeletes == 0);
    SetMuiLanguage(MuiLanguages_CountOf());
    CHECK(wcscmp(Globals.CurrentLngLocaleName, L"en-US") == 0);
    CHECK(iniDeletes == 1 && iniSets == 0);
    SetMuiLanguage(german);
    CHECK(iniDeletes == 1 && iniSets == 1);
    if (failures) return 1;
    puts("MuiLanguage C-caller tests passed.");
    return 0;
}
