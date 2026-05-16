// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* EditRTF.cpp                                                                 *
*   Copy current selection (or whole document) to clipboard as RTF,           *
*   preserving Scintilla syntax highlighting.                                 *
*                                                                             *
*   The RTF builder is a port of Notepad4's SaveToStreamRTF                   *
*   (notepad4/src/Bridge.cpp), which itself derives from SciTE's              *
*   ExportRTF.cxx. Adapted to Notepad3 globals (Globals.fvCurFile,            *
*   g_Encodings[CPI_ANSI_DEFAULT]) and to upstream Scintilla's interleaved    *
*   SCI_GETSTYLEDTEXTFULL layout. UTF-8 decoding is done inline because       *
*   Notepad3's vendored Scintilla does not expose SCI_GETCHARACTERANDWIDTH.   *
*                                                                             *
*   The clipboard transaction sets both CF_RTF and CF_UNICODETEXT so styled   *
*   consumers (Word/WordPad/OneNote/Outlook) and plain consumers (Notepad,    *
*   chat apps, terminals) both work.                                          *
*                                                                             *
*   Per-line \cbpat paragraph background is emitted only when every visible   *
*   char on the line shares the same eolFilled background (markdown header    *
*   lines, uniformly-styled code-block lines). Mixed-style lines fall back    *
*   to char-only \highlight so a trailing inline-code span never paints       *
*   the whole line.                                                           *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#include <sdkddkver.h>
#if !defined(WINVER)
#define WINVER _WIN32_WINNT_WIN10
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION NTDDI_WIN10_RS5
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

extern "C" {
#include "Scintilla.h"
#include "SciCall.h"
#include "TypeDefs.h"
#include "Notepad3.h"
#include "Encoding.h"
}
#include "EditRTF.h"

namespace {

// ---- RTF control words (from Notepad4 Bridge.cpp) ---------------------------

#define RTF_HEADEROPEN     "{\\rtf1\\ansi\\ansicpg%u\\deff0\\uc1\\deftab%d"
#define RTF_FONTDEFOPEN    "{\\fonttbl"
#define RTF_FONTDEFCLOSE   "}"
#define RTF_COLORDEFOPEN   "{\\colortbl;"
#define RTF_COLORDEFCLOSE  "}"
#define RTF_HEADERCLOSE    "\n"
// Per-paragraph properties (\pard\cbpat?\sb0\sa0) are emitted in the per-line
// loop, not in the body opener, so eolFilled styles can paint the whole line.
#define RTF_BODYOPEN       ""
#define RTF_BODYCLOSE      "}"
#define RTF_SETFONTFACE    "\\f"
#define RTF_SETFONTSIZE    "\\fs"
#define RTF_SETCOLOR       "\\cf"
#define RTF_SETBACKGROUND  "\\highlight"
#define RTF_BOLD_ON        "\\b"
#define RTF_BOLD_OFF       "\\b0"
#define RTF_ITALIC_ON      "\\i"
#define RTF_ITALIC_OFF     "\\i0"
#define RTF_UNDERLINE_ON   "\\ul"
#define RTF_UNDERLINE_OFF  "\\ulnone"
#define RTF_STRIKE_ON      "\\strike"
#define RTF_STRIKE_OFF     "\\strike0"
#define RTF_PAR            "\\par\n"
#define RTF_TAB            "\\tab "

// font face, size, color, bold, italic, underline, strike, highlight
constexpr int RTF_MAX_STYLEPROP = 8;
constexpr int RTF_MAX_STYLEDEF  = 128;

// ---- Style snapshot --------------------------------------------------------

struct StyleDefinition {
    int       fontSize;
    COLORREF  foreColor;
    COLORREF  backColor;
    int       weight;
    bool      italic;
    bool      underline;
    bool      strike;
    bool      eolFilled;
    int       charset;
    WCHAR     fontWide[LF_FACESIZE];
    char      fontFace[LF_FACESIZE];
    uint16_t  backIndex;
};

static_assert(__is_standard_layout(StyleDefinition));

void GetStyleDefinitionFor(int style, StyleDefinition &d) noexcept
{
    d.fontSize  = SciCall_StyleGetSizeFractional(style);
    d.foreColor = SciCall_StyleGetFore(style);
    d.backColor = SciCall_StyleGetBack(style);
    d.weight    = SciCall_StyleGetWeight(style);
    d.italic    = SciCall_StyleGetItalic(style);
    d.underline = SciCall_StyleGetUnderline(style);
    d.strike    = SciCall_StyleGetStrike(style);
    // EOL-fill drives whole-paragraph background (\cbpat) so e.g. markdown
    // header lines paint edge-to-edge in the pasted RTF, not just behind the
    // visible characters.
    d.eolFilled = SciCall_StyleGetEOLFilled(style);
    d.charset   = SciCall_StyleGetCharacterSet(style);
    d.backIndex = 0;
    memset(d.fontFace, 0, sizeof(d.fontFace));
    memset(d.fontWide, 0, sizeof(d.fontWide));
    SciCall_StyleGetFont(style, d.fontFace);
}

struct DocumentStyledText {
    std::unique_ptr<StyleDefinition[]> styleList;
    unsigned styleCount;
    UINT     cpEdit;
};

DocumentStyledText GetDocumentStyledText(uint8_t (&styleMap)[STYLE_MAX + 1],
                                         const unsigned char *styles,
                                         size_t numChars) noexcept
{
    uint32_t styleUsed[8]{};
    styleUsed[STYLE_DEFAULT >> 5] |= (1U << (STYLE_DEFAULT & 31));
    unsigned maxStyle = STYLE_DEFAULT;

    for (size_t i = 0; i < numChars; i++) {
        const uint8_t s = styles[i];
        styleUsed[s >> 5] |= (1U << (s & 31));
        if (s > maxStyle) {
            maxStyle = s;
        }
    }
    ++maxStyle;

    memset(styleMap, 0, STYLE_MAX + 1);

    auto styleList = std::make_unique<StyleDefinition[]>(maxStyle);
    unsigned styleCount = 0;

    // STYLE_DEFAULT (== 32) always sits at index 0, since it is the implicit
    // baseline that following style deltas are computed against.
    if constexpr (STYLE_DEFAULT != 0) {
        styleCount = 1;
        styleUsed[STYLE_DEFAULT >> 5] &= ~(1U << (STYLE_DEFAULT & 31));
        GetStyleDefinitionFor(STYLE_DEFAULT, styleList[0]);
        styleMap[STYLE_DEFAULT] = 0;
    }

    for (unsigned style = 0; style < maxStyle; style++) {
        if (!(styleUsed[style >> 5] & (1U << (style & 31)))) {
            continue;
        }
        StyleDefinition &def = styleList[styleCount];
        GetStyleDefinitionFor(style, def);
        unsigned idx = 0;
        for (; idx < styleCount; idx++) {
            if (memcmp(&def, &styleList[idx], sizeof(StyleDefinition)) == 0) {
                memset(def.fontFace, 0, sizeof(def.fontFace));
                break;
            }
        }
        styleMap[style] = static_cast<uint8_t>(idx);
        if (idx == styleCount) {
            styleCount++;
        }
    }

    const UINT cpEdit = static_cast<UINT>(SciCall_GetCodePage());
    return { std::move(styleList), styleCount, cpEdit };
}

// ---- RTF style-delta helpers -----------------------------------------------

void GetRTFNextControl(const char **style, char *control) noexcept
{
    const char *pos = *style;
    *control = '\0';
    if ('\0' == *pos) {
        return;
    }
    pos++; // implicit skip over leading '\'
    while ('\0' != *pos && '\\' != *pos) {
        pos++;
    }
    const size_t len = pos - *style;
    memcpy(control, *style, len);
    control[len] = '\0';
    *style = pos;
}

void GetRTFStyleChange(std::string &delta, const char *last, const char *current)
{
    char lastControl[RTF_MAX_STYLEDEF];
    char currentControl[RTF_MAX_STYLEDEF];
    lastControl[0] = '\0';
    currentControl[0] = '\0';
    const char *lastPos = last;
    const char *currentPos = current;
    const size_t prevLen = delta.length();
    for (int i = 0; i < RTF_MAX_STYLEPROP; i++) {
        GetRTFNextControl(&lastPos, lastControl);
        GetRTFNextControl(&currentPos, currentControl);
        if (strcmp(lastControl, currentControl) != 0) {
            delta += currentControl;
        }
    }
    if (prevLen != delta.length()) {
        delta += ' ';
    }
}

constexpr int GetRTFFontSize(int sizeFractional) noexcept
{
    return sizeFractional / (SC_FONT_SIZE_MULTIPLIER / 2);
}

// Decode one UTF-8 codepoint starting at p (with `available` bytes remaining
// in the source buffer). Sets `width` to bytes consumed (1..4). On a
// truncated multi-byte sequence, falls back to emitting the lead byte raw.
uint32_t DecodeUtf8(const char *p, size_t available, int &width) noexcept
{
    if (available == 0) {
        width = 1;
        return 0;
    }
    const unsigned char c0 = static_cast<unsigned char>(p[0]);
    if (c0 < 0x80 || available < 2) {
        width = 1;
        return c0;
    }
    if ((c0 & 0xE0) == 0xC0) {
        width = 2;
        return ((c0 & 0x1Fu) << 6)
             |  (static_cast<unsigned char>(p[1]) & 0x3Fu);
    }
    if ((c0 & 0xF0) == 0xE0 && available >= 3) {
        width = 3;
        return ((c0 & 0x0Fu) << 12)
             | ((static_cast<unsigned char>(p[1]) & 0x3Fu) << 6)
             |  (static_cast<unsigned char>(p[2]) & 0x3Fu);
    }
    if (available >= 4) {
        width = 4;
        return ((c0 & 0x07u) << 18)
             | ((static_cast<unsigned char>(p[1]) & 0x3Fu) << 12)
             | ((static_cast<unsigned char>(p[2]) & 0x3Fu) << 6)
             |  (static_cast<unsigned char>(p[3]) & 0x3Fu);
    }
    // Truncated multi-byte sequence — emit the lead byte raw.
    width = 1;
    return c0;
}

// ---- Main RTF builder ------------------------------------------------------

void SaveToStreamRTF(std::string &os,
                     const unsigned char *styles,
                     const char *textBuffer,
                     size_t numChars)
{
    uint8_t styleMap[STYLE_MAX + 1];
    auto sdt = GetDocumentStyledText(styleMap, styles, numChars);
    StyleDefinition *styleList = sdt.styleList.get();
    const unsigned styleCount  = sdt.styleCount;
    const UINT cpEdit          = sdt.cpEdit;

    auto stylesText = std::make_unique<std::string[]>(styleCount);
    auto fontList   = std::make_unique<LPCSTR[]>(styleCount);
    auto colorList  = std::make_unique<COLORREF[]>(2 * styleCount + 1);

    UINT legacyACP = cpEdit;
    if (legacyACP == SC_CP_UTF8 || legacyACP == 0) {
        legacyACP = g_Encodings[CPI_ANSI_DEFAULT].uCodePage;
    }

    // Reader-default tab width in twips. ~120 twips per char at 10pt monospace
    // (one char is roughly 8pt wide → 120 twips). Only matters when tabs are
    // emitted as RTF \tab tokens (i.e. bTabsAsSpaces is false); when we expand
    // tabs to spaces ourselves, the reader's \deftab is unused.
    const int defTabTwips = (Globals.fvCurFile.iTabWidth > 0)
                                ? Globals.fvCurFile.iTabWidth * 120
                                : 480;
    char fmtbuf[RTF_MAX_STYLEDEF];
    unsigned fmtlen = snprintf(fmtbuf, sizeof(fmtbuf),
                               RTF_HEADEROPEN RTF_FONTDEFOPEN, legacyACP, defTabTwips);
    os += std::string_view{fmtbuf, fmtlen};

    // STYLE_DEFAULT is always laid out at styleList[0] by GetDocumentStyledText.
    // Its backColor is the document's baseline page background; styles matching
    // it get \highlight0 (no marker), styles differing get a \highlight<n>
    // pulled from the color table.
    const COLORREF defaultBack = (styleCount > 0) ? styleList[0].backColor : 0;

    int fontCount = 0;
    int colorCount = 0;
    for (unsigned styleIndex = 0; styleIndex < styleCount; styleIndex++) {
        StyleDefinition &d = styleList[styleIndex];

        int iFont = 0;
        for (; iFont < fontCount; iFont++) {
            if (_stricmp(fontList[iFont], d.fontFace) == 0) {
                break;
            }
        }
        if (iFont == fontCount) {
            fontList[fontCount++] = d.fontFace;
            char fontFaceACP[LF_FACESIZE]{};
            MultiByteToWideChar(CP_UTF8, 0, d.fontFace, -1, d.fontWide,
                                static_cast<int>(_countof(d.fontWide)));
            WideCharToMultiByte(legacyACP, 0, d.fontWide, -1, fontFaceACP,
                                static_cast<int>(_countof(fontFaceACP)), nullptr, nullptr);
            const int charset = d.charset;
            if (charset == DEFAULT_CHARSET || charset == ANSI_CHARSET) {
                fmtlen = snprintf(fmtbuf, sizeof(fmtbuf),"{\\f%d\\fnil %s;}", iFont, fontFaceACP);
            } else {
                fmtlen = snprintf(fmtbuf, sizeof(fmtbuf),"{\\f%d\\fnil\\fcharset%d %s;}", iFont, charset, fontFaceACP);
            }
            os += std::string_view{fmtbuf, fmtlen};
        }

        int iFore = 0;
        for (; iFore < colorCount; iFore++) {
            if (colorList[iFore] == d.foreColor) {
                break;
            }
        }
        if (iFore == colorCount) {
            colorList[colorCount++] = d.foreColor;
        }

        // Only register a background colour if this style actually deviates
        // from the page baseline; baseline-matching styles get \highlight0.
        unsigned highlightIdx = 0;
        if (d.backColor != defaultBack) {
            int iBack = 0;
            for (; iBack < colorCount; iBack++) {
                if (colorList[iBack] == d.backColor) {
                    break;
                }
            }
            if (iBack == colorCount) {
                colorList[colorCount++] = d.backColor;
            }
            highlightIdx = static_cast<unsigned>(iBack + 1);
        }
        d.backIndex = static_cast<uint16_t>(highlightIdx);

        fmtlen = snprintf(fmtbuf, sizeof(fmtbuf),RTF_SETFONTFACE "%d" RTF_SETFONTSIZE "%d" RTF_SETCOLOR "%d",
                         iFont, GetRTFFontSize(d.fontSize), iFore + 1);

        std::string osStyle(fmtbuf, fmtlen);
        osStyle += (d.weight >= FW_SEMIBOLD ? RTF_BOLD_ON      : RTF_BOLD_OFF);
        osStyle += (d.italic                 ? RTF_ITALIC_ON    : RTF_ITALIC_OFF);
        osStyle += (d.underline              ? RTF_UNDERLINE_ON : RTF_UNDERLINE_OFF);
        osStyle += (d.strike                 ? RTF_STRIKE_ON    : RTF_STRIKE_OFF);
        fmtlen = snprintf(fmtbuf, sizeof(fmtbuf), RTF_SETBACKGROUND "%u", highlightIdx);
        osStyle.append(fmtbuf, fmtlen > 0 ? static_cast<size_t>(fmtlen) : 0);
        stylesText[styleIndex] = std::move(osStyle);
    }

    os += RTF_FONTDEFCLOSE RTF_COLORDEFOPEN;
    for (int i = 0; i < colorCount; i++) {
        const COLORREF color = colorList[i];
        fmtlen = snprintf(fmtbuf, sizeof(fmtbuf),"\\red%d\\green%d\\blue%d;",
                         static_cast<int>(color & 0xff),
                         static_cast<int>((color >> 8) & 0xff),
                         static_cast<int>((color >> 16) & 0xff));
        os += std::string_view{fmtbuf, fmtlen};
    }
    os += RTF_COLORDEFCLOSE RTF_HEADERCLOSE RTF_BODYOPEN;

    // Returns the colortbl index to use for \cbpat (paragraph background) on
    // the line beginning at `startOffset`. Requires every visible char on the
    // line to share the same eolFilled background; otherwise the line has at
    // least one span that wants char-only fill and a \cbpat would over-paint.
    // 0 means "no paragraph fill" (Word's page background shows through).
    auto getLineCbpat = [&](size_t startOffset) noexcept -> unsigned {
        if (startOffset >= numChars) {
            return 0;
        }
        size_t end = startOffset;
        while (end < numChars && textBuffer[end] != '\r' && textBuffer[end] != '\n') {
            end++;
        }
        if (end == startOffset) {
            return 0; // empty line
        }
        const uint8_t sFirst = styleMap[styles[startOffset]];
        if (!styleList[sFirst].eolFilled) {
            return 0;
        }
        const COLORREF bg = styleList[sFirst].backColor;
        for (size_t i = startOffset + 1; i < end; i++) {
            const uint8_t s = styleMap[styles[i]];
            if (!styleList[s].eolFilled || styleList[s].backColor != bg) {
                return 0;
            }
        }
        return styleList[sFirst].backIndex;
    };

    auto emitParagraphOpen = [&](unsigned cbpatIdx) {
        if (cbpatIdx > 0) {
            const int n = snprintf(fmtbuf, sizeof(fmtbuf),
                                   "\\pard\\cbpat%u\\sb0\\sa0 ", cbpatIdx);
            if (n > 0) {
                os.append(fmtbuf, static_cast<size_t>(n));
            }
        } else {
            os += "\\pard\\sb0\\sa0 ";
        }
    };

    emitParagraphOpen(getLineCbpat(0));

    const char *lastStyle = "";
    unsigned styleCurrent = STYLE_MAX + 1;
    unsigned column = 0;

    for (size_t offset = 0; offset < numChars; offset++) {
        uint8_t style = styles[offset];
        style = styleMap[style];
        if (style != styleCurrent) {
            styleCurrent = style;
            const char *currentStyle = stylesText[style].c_str();
            GetRTFStyleChange(os, lastStyle, currentStyle);
            lastStyle = currentStyle;
        }

        const char ch = textBuffer[offset];
        std::string_view sv;
        column++;

        if (ch == '\t') {
            if (!Globals.fvCurFile.bTabsAsSpaces) {
                sv = RTF_TAB;
            } else {
                const unsigned tabWidth = static_cast<unsigned>(Globals.fvCurFile.iTabWidth);
                const unsigned padding = tabWidth - ((column - 1) % tabWidth);
                column += padding;
                os.append(padding, ' ');
            }
        } else if (ch == '\r' || ch == '\n') {
            os += RTF_PAR;
            column = 0;
            if (ch == '\r' && (offset + 1) < numChars && textBuffer[offset + 1] == '\n') {
                offset += 1;
            }
            // Open the next paragraph (if any) with its own cbpat state.
            if (offset + 1 < numChars) {
                emitParagraphOpen(getLineCbpat(offset + 1));
            }
            continue;
        } else if (static_cast<signed char>(ch) < 0 && cpEdit == SC_CP_UTF8) {
            int width = 1;
            const uint32_t u32 = DecodeUtf8(textBuffer + offset, numChars - offset, width);
            offset += static_cast<size_t>(width - 1);
            if (u32 < 0x10000) {
                fmtlen = snprintf(fmtbuf, sizeof(fmtbuf),"\\u%d?", static_cast<short>(u32));
            } else {
                fmtlen = snprintf(fmtbuf, sizeof(fmtbuf),"\\u%d?\\u%d?",
                                 static_cast<short>(((u32 - 0x10000) >> 10) + 0xD800),
                                 static_cast<short>((u32 & 0x3ff) + 0xDC00));
            }
            sv = std::string_view{fmtbuf, fmtlen};
        }

        if (sv.empty()) {
            if (ch != '\t') {
                if (ch == '{' || ch == '}' || ch == '\\') {
                    os += '\\';
                }
                os += ch;
            }
        } else {
            os += sv;
        }
    }

    os += RTF_BODYCLOSE;
}

// ---- Clipboard helpers -----------------------------------------------------

// Allocate a movable global block and copy `len` bytes (no NUL-terminator
// management; caller passes the desired payload length including any NUL).
HGLOBAL AllocClipboardBlock(const void *data, size_t bytes) noexcept
{
    HGLOBAL h = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
    if (!h) {
        return nullptr;
    }
    void *p = ::GlobalLock(h);
    if (!p) {
        ::GlobalFree(h);
        return nullptr;
    }
    memcpy(p, data, bytes);
    ::GlobalUnlock(h);
    return h;
}

} // namespace

// ---- Public entry point ----------------------------------------------------

extern "C" void EditCopyAsRTF(HWND hwnd)
{
    try {
    DocPos startPos = SciCall_GetSelectionStart();
    DocPos endPos   = SciCall_GetSelectionEnd();
    if (startPos == endPos) {
        startPos = 0;
        endPos   = SciCall_GetLength();
        if (endPos == 0) {
            return;
        }
    }
    if (endPos < startPos) {
        std::swap(startPos, endPos);
    }

    const DocPos numCharsPos = endPos - startPos;
    if (numCharsPos <= 0) {
        return;
    }
    const size_t numChars = static_cast<size_t>(numCharsPos);

    // Make sure styling is computed for the range.
    SciCall_Colourise(startPos, endPos);

    // Retrieve interleaved [c0,s0,c1,s1,...,cn-1,sn-1,0,0] from upstream Scintilla.
    auto interleaved = std::make_unique<char[]>(2 * numChars + 2);
    Sci_TextRangeFull tr{};
    tr.chrg.cpMin = startPos;
    tr.chrg.cpMax = endPos;
    tr.lpstrText  = interleaved.get();
    SciCall_GetStyledTextFull(&tr);

    // De-interleave so the RTF builder can address chars and styles by char index.
    // make_unique<T[]> value-initialises, so the trailing terminator slot is
    // already zero — no explicit '\0' write needed.
    auto chars  = std::make_unique<char[]>(numChars + 1);
    auto styles = std::make_unique<unsigned char[]>(numChars + 1);
    for (size_t i = 0; i < numChars; i++) {
        chars[i]  = interleaved[2 * i];
        styles[i] = static_cast<unsigned char>(interleaved[2 * i + 1]);
    }
    interleaved.reset();

    // Build RTF. (Outer try/catch in EditCopyAsRTF handles any allocation
    // failure; we don't need a separate inner guard here.)
    std::string rtf;
    rtf.reserve(numChars * 2);
    SaveToStreamRTF(rtf, styles.get(), chars.get(), numChars);

    // Build UTF-16 plain-text companion. make_unique<WCHAR[]> value-initialises,
    // so the trailing L'\0' is already in place.
    const int wlen = ::MultiByteToWideChar(CP_UTF8, 0, chars.get(),
                                           static_cast<int>(numChars), nullptr, 0);
    std::unique_ptr<WCHAR[]> wbuf;
    if (wlen > 0) {
        wbuf = std::make_unique<WCHAR[]>(static_cast<size_t>(wlen) + 1);
        ::MultiByteToWideChar(CP_UTF8, 0, chars.get(),
                              static_cast<int>(numChars), wbuf.get(), wlen);
    }

    // Push both formats in one clipboard transaction.
    HWND owner = hwnd ? hwnd : Globals.hwndMain;
    if (!::OpenClipboard(owner)) {
        return;
    }

    HGLOBAL hRTF = AllocClipboardBlock(rtf.c_str(), rtf.length() + 1);
    HGLOBAL hTxt = nullptr;
    if (wbuf) {
        hTxt = AllocClipboardBlock(wbuf.get(),
                                   (static_cast<size_t>(wlen) + 1) * sizeof(WCHAR));
    }

    if (hRTF || hTxt) {
        ::EmptyClipboard();
        if (hRTF) {
            UINT cfRTF = ::RegisterClipboardFormatW(L"Rich Text Format");
            if (cfRTF) {
                if (!::SetClipboardData(cfRTF, hRTF)) {
                    ::GlobalFree(hRTF);
                }
            } else {
                ::GlobalFree(hRTF);
            }
        }
        if (hTxt) {
            if (!::SetClipboardData(CF_UNICODETEXT, hTxt)) {
                ::GlobalFree(hTxt);
            }
        }
    }
    ::CloseClipboard();
    } catch (...) {
        // Swallow std::bad_alloc and any other C++ exceptions so they never
        // unwind into the C caller (UB on the Win32 C-ABI dispatch path).
    }
}
