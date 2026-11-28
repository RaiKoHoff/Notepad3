#include <stdio.h>
#include <string.h>

#include "Helpers.h"
#include "Encoding.h"

GLOBALS_T Globals;
SETTINGS_T Settings;

NP2ENCODING g_Encodings[] = {
    { NCP_DEFAULT | NCP_ANSI, 1252, ",ansi,", 0, L"" },
    { NCP_OEM, 437, ",oem,", 0, L"" },
    { NCP_UNICODE | NCP_UNICODE_BOM, 1200, ",utf-16le-bom,", 0, L"" },
    { NCP_UNICODE | NCP_UNICODE_BOM | NCP_UNICODE_REVERSE, 1201, ",utf-16be-bom,", 0, L"" },
    { NCP_UNICODE, 1200, ",utf-16le,", 0, L"" },
    { NCP_UNICODE | NCP_UNICODE_REVERSE, 1201, ",utf-16be,", 0, L"" },
    { NCP_UTF8, 65001, ",utf-8,utf8,", 0, L"" },
    { NCP_UTF8 | NCP_UTF8_SIGN, 65001, ",utf-8-sig,", 0, L"" },
    { NCP_INTERNAL, 65000, ",utf-7,", 0, L"" },
    { NCP_EXTERNAL_8BIT, 1252, ",windows-1252,", 0, L"" },
    { NCP_EXTERNAL_8BIT, 99999, ",invalid-cp,", 0, L"" }
};

cpi_enc_t Encoding_CountOf(void) { return (cpi_enc_t)COUNTOF(g_Encodings); }
int Scintilla_InputCodePage(void) { return 0; }
UINT CodePageFromCharSet(const UINT charset) { (void)charset; return 0; }
void ChangeEncodingCodePage(const cpi_enc_t cpi, UINT cp) { (void)cpi; (void)cp; }
int LoadLngStringW(UINT id, LPWSTR buffer, int size) { (void)id; (void)buffer; (void)size; return 0; }
int timsort(void* base, size_t count, size_t width, int (*compare)(const void*, const void*))
{
    (void)base; (void)count; (void)width; (void)compare;
    return 0;
}

static int failures;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); ++failures; } } while (0)

static void CheckUTF8(const char* bytes, size_t length, bool valid, bool ascii, bool hasNull, bool withNull)
{
    bool actualASCII = false, actualNull = false, actualWithNull = false;
    CHECK(IsValidUTF8(bytes, length, &actualASCII, &actualNull, &actualWithNull) == valid);
    CHECK(actualASCII == ascii);
    CHECK(actualNull == hasNull);
    CHECK(actualWithNull == withNull);
}

int main(void)
{
    static const char utf32le[] = "\xff\xfe\x00\x00";
    static const char utf32be[] = "\x00\x00\xfe\xff";
    static const char utf16be[] = "\xfe\xff";
    static const char withNull[] = { 'a', '\0', 'b' };
    static const char invalid[] = "\xc0\x80\x00";
    static const unsigned char original[] = { 0, 1, 2, 0xfd, 0xfe, 0xff };
    size_t encodedLen = 0, decodedLen = 0;

    Globals.hndlProcessHeap = GetProcessHeap();
    Settings.DefaultEncoding = CPI_UTF8;
    CHECK(Encoding_MapIniSetting(true, 3) == CPI_UTF8);
    CHECK(Encoding_MapIniSetting(false, CPI_UTF8SIGN) == 4);
    CHECK(Encoding_MapIniSetting(true, 1252) == 9);
    CHECK(Encoding_MapIniSetting(false, 9) == 1252);
    CHECK(Encoding_MapIniSetting(true, 99999) == CPI_UTF8);
    CHECK(Encoding_MapIniSetting(false, 999) == CPI_UTF8);
    CHECK(Encoding_MapSignature(CPI_UNICODEBOM) == CPI_UNICODE);
    CHECK(Encoding_MapSignature(CPI_UNICODEBEBOM) == CPI_UNICODEBE);
    CHECK(Encoding_MapSignature(CPI_UTF8SIGN) == CPI_UTF8);
    CHECK(Encoding_MapSignature(CPI_UTF8) == CPI_UTF8);
    CHECK(Encoding_MatchA("UTF8") == CPI_UTF8);
    CHECK(Encoding_MatchA("windows-1252") == 9);
    CHECK(Encoding_MatchA("invalid-cp") == CPI_NONE);
    CHECK(Encoding_MatchA("unknown") == CPI_NONE);
    CHECK(Encoding_IsValid(CPI_UTF8));
    CHECK(!Encoding_IsValid(CPI_NONE));
    CHECK(!Encoding_IsValid(10));
    CHECK(Encoding_GetWCMBFlagsByCodePage(65001) == 0);
    CHECK(Encoding_GetWCMBFlagsByCodePage(1252) == WC_NO_BEST_FIT_CHARS);
    CHECK(Encoding_IsUNICODE_BOM(CPI_UNICODEBEBOM));
    CHECK(Encoding_IsUNICODE_REVERSE(CPI_UNICODEBE));
    CHECK(Encoding_MaybeUTF8_NO_SIGN(CPI_UTF8));
    CHECK(!Encoding_MaybeUTF8_NO_SIGN(CPI_UTF8SIGN));
    CHECK(Has_UTF32_LE_BOM(utf32le, 4));
    CHECK(!Has_UTF16_LE_BOM(utf32le, 4));
    CHECK(Has_UTF16_LE_BOM(utf32le, 2));
    CHECK(Has_UTF32_BE_BOM(utf32be, 4));
    CHECK(Has_UTF16_BE_BOM(utf16be, 2));
    CHECK(!Has_UTF32_BOM(utf32le, 3));
    CHECK(!Has_UTF16_BOM(NULL, 4));
    CheckUTF8("abcdefgh", 8, true, true, false, false);
    CheckUTF8("\xc3\xa9", 2, true, false, false, false);
    CheckUTF8(withNull, sizeof(withNull), false, false, true, true);
    CheckUTF8(invalid, sizeof(invalid) - 1, false, false, true, false);
    CheckUTF8("\xe2\x82", 2, false, false, false, false);

    unsigned char* encoded = Encoding_Base64Encode(original, sizeof(original), &encodedLen);
    CHECK(encoded && encodedLen == 8 && memcmp(encoded, "AAEC/f7/", 8) == 0);
    if (encoded) {
        unsigned char* decoded = Encoding_Base64Decode(encoded, encodedLen, &decodedLen);
        CHECK(decoded && decodedLen == sizeof(original) && memcmp(decoded, original, sizeof(original)) == 0);
        if (decoded) FreeMem(decoded);
        FreeMem(encoded);
    }
    if (failures) return 1;
    puts("Encoding unit C-caller tests passed.");
    return 0;
}
