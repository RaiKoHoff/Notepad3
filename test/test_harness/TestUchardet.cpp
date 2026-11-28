#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "uchardet/uchardet/src/uchardet.h"
#include "uchardet/uchardet/src/nscore.h"
#include "uchardet/uchardet/src/nsUniversalDetector.h"

using cpi_enc_t = int;
constexpr cpi_enc_t CPI_NONE = -1;

struct {
    unsigned int UchardetLanguageFilter;
} Settings2;

static float clampf(float value, float low, float high)
{
    return (std::min)((std::max)(value, low), high);
}

static cpi_enc_t _MapStdEncodingString2CPI(const char* charset, float* confidence)
{
    if (!charset || !*charset) {
        *confidence = 0.0f;
        return CPI_NONE;
    }
    unsigned int value = 0;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(charset); *p; ++p) {
        value = (value * 31 + *p) & 0x7fffffff;
    }
    return static_cast<cpi_enc_t>(value);
}

static bool failReportCopy = false;
static char* TestCharsetDup(const char* charset)
{
    return failReportCopy ? nullptr : _strdup(charset);
}

#define _strdup TestCharsetDup
#include "UchardetProduction.inc"
#undef _strdup

static int detectedCount = 0;
static int needMoreCount = 0;
static int emptyCount = 0;

static bool Check(const char* label, const std::string& sample, unsigned int filter, int capacity)
{
    char expectedName[128] = "unchanged";
    char actualName[128] = "unchanged";
    float expectedConfidence = 0.0f;
    uchardet_t oldDetector = uchardet_new();
    uchardet_set_language_filter(oldDetector, filter);
    const int status = uchardet_handle_data(oldDetector, sample.data(), sample.size());
    uchardet_data_end(oldDetector);

    detectedCount += status == HANDLE_DATA_RESULT_DETECTED;
    needMoreCount += status == HANDLE_DATA_RESULT_NEED_MORE_DATA;
    emptyCount += !*uchardet_get_charset(oldDetector);

    cpi_enc_t expectedEncoding = CPI_NONE;
    if (status == HANDLE_DATA_RESULT_DETECTED || status == HANDLE_DATA_RESULT_NEED_MORE_DATA) {
        const char* charset = uchardet_get_charset(oldDetector);
        StringCchCopyA(expectedName, capacity, charset);
        expectedConfidence = uchardet_get_confidence(oldDetector);
        expectedEncoding = _MapStdEncodingString2CPI(charset, &expectedConfidence);
    }
    uchardet_delete(oldDetector);
    expectedConfidence = clampf(expectedConfidence, 0.0f, 1.0f);

    Settings2.UchardetLanguageFilter = filter;
    float actualConfidence = -1.0f;
    const cpi_enc_t actualEncoding = AnalyzeText_UCHARDET(
        sample.data(), sample.size(), &actualConfidence, actualName, capacity);

    if (actualEncoding != expectedEncoding || actualConfidence != expectedConfidence ||
        std::strcmp(actualName, expectedName) != 0) {
        std::fprintf(stderr, "%s filter=%u capacity=%d: expected %d %.9g '%s', got %d %.9g '%s'\n",
            label, filter, capacity, expectedEncoding, expectedConfidence, expectedName,
            actualEncoding, actualConfidence, actualName);
        return false;
    }
    return true;
}

int main()
{
    struct Sample {
        const char* label;
        std::string bytes;
    };
    const Sample samples[] = {
        {"empty", {}},
        {"ascii", "Plain ASCII text.\n"},
        {"nbsp", std::string("word\xa0word", 9)},
        {"utf8", "Gr\xc3\xbc\xc3\x9f" "e \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e"},
        {"utf8-bom", "\xef\xbb\xbf" "hello"},
        {"utf16-bom", std::string("\xff\xfe" "a\0", 4)},
        {"latin1", "Caf\xe9 et cr\xe8me, d\xe9j\xe0 vu."},
        {"shift-jis", "\x93\xfa\x96\x7b\x8c\xea\x82\xcc\x83\x65\x83\x58\x83\x67"},
        {"iso-2022", "\x1b$B$3$s$K$A$O\x1b(B"},
        {"hz", "~{VPND~}"},
        {"uncertain", std::string("\x80", 1)},
    };
    const unsigned int filters[] = {
        UCHARDET_FILTER_ALL, UCHARDET_FILTER_NON_CJK, UCHARDET_FILTER_JAPANESE,
        UCHARDET_FILTER_CHINESE_SIMPLIFIED, UCHARDET_FILTER_KOREAN, 0
    };
    int count = 0;
    for (const Sample& sample : samples) {
        for (unsigned int filter : filters) {
            for (int capacity : {1, 8, 128}) {
                if (!Check(sample.label, sample.bytes, filter, capacity)) {
                    return 1;
                }
                ++count;
            }
        }
    }
    if (!detectedCount || !needMoreCount || !emptyCount) {
        std::fprintf(stderr, "Missing status or empty-result coverage: %d %d %d\n",
            detectedCount, needMoreCount, emptyCount);
        return 1;
    }

    char name[128] = "unchanged";
    float confidence = -1.0f;
    Settings2.UchardetLanguageFilter = UCHARDET_FILTER_ALL;
    failReportCopy = true;
    const cpi_enc_t failedCopy = AnalyzeText_UCHARDET("ASCII", 5, &confidence, name, sizeof(name));
    failReportCopy = false;
    if (failedCopy != CPI_NONE || confidence != 0.0f || *name) {
        std::fprintf(stderr, "Failed charset copy did not produce an empty, zero-confidence result\n");
        return 1;
    }
    std::printf("uchardet real-model parity: %d cases passed\n", count);
    return 0;
}
