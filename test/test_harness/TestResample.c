#include <stdio.h>
#include <string.h>

#include "..\..\src\Resample.h"

GLOBALS_T Globals = { 0 };

static int failures;
static int checks;
static int customCalls;

#define CHECK(condition) do { \
    ++checks; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

static double TriangleFilter(double x)
{
    ++customCalls;
    if (x < 0.0) {
        x = -x;
    }
    return x < 1.0 ? 1.0 - x : 0.0;
}

static BITMAPINFO BitmapInfo(int width, int height)
{
    BITMAPINFO info = { 0 };
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    return info;
}

static BOOL ReadPixels(HDC dc, HBITMAP bitmap, int width, int height, DWORD* pixels)
{
    BITMAPINFO info = BitmapInfo(width, height);
    return GetDIBits(dc, bitmap, 0, (UINT)height, pixels, &info, DIB_RGB_COLORS) == height;
}

static BOOL Dimensions(HBITMAP bitmap, int width, int height)
{
    BITMAP info = { 0 };
    return GetObject(bitmap, sizeof(info), &info) == sizeof(info) &&
        info.bmWidth == width && info.bmHeight == height;
}

int main(void)
{
    static const DWORD sourcePixels[] = { 0x00102030, 0x00f06020, 0x0020d080, 0x00c040f0 };
    static const DWORD lanczos8Pixels[] = {
        0x00102030, 0x00a54a25, 0x00ff741a,
        0x001a9565, 0x00926293, 0x00f632b3,
        0x0025ff99, 0x00817afd, 0x00ed00ff
    };
    static const DWORD trianglePixels[] = {
        0x00102030, 0x00a54a25, 0x00f05f20,
        0x001a9565, 0x00936393, 0x00cf4aaa,
        0x0020d080, 0x008a70ca, 0x00bf40f0
    };
    DWORD pixels[9] = { 0 };
    DWORD wrappedPixels[9] = { 0 };
    BITMAPINFO info = BitmapInfo(2, 2);
    void* referenceBits = NULL;
    void* sourceBits = NULL;
    Globals.hndlProcessHeap = GetProcessHeap();

    HDC dc = CreateCompatibleDC(NULL);
    CHECK(dc != NULL);
    if (!dc) {
        return 1;
    }
    HBITMAP reference = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &referenceBits, NULL, 0);
    HBITMAP source = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &sourceBits, NULL, 0);
    CHECK(reference != NULL && source != NULL && referenceBits != NULL && sourceBits != NULL);
    if (!reference || !source || !referenceBits || !sourceBits) {
        if (reference) { DeleteObject(reference); }
        if (source) { DeleteObject(source); }
        DeleteDC(dc);
        return 1;
    }
    HGDIOBJ oldBitmap = SelectObject(dc, reference);
    CHECK(oldBitmap != NULL && oldBitmap != HGDI_ERROR);
    memcpy(sourceBits, sourcePixels, sizeof(sourcePixels));

    HBITMAP same = CreateResampledBitmap(dc, source, 2, 2, STOCK_FILTER_LANCZOS8);
    CHECK(same != NULL);
    if (same) {
        CHECK(Dimensions(same, 2, 2));
        CHECK(ReadPixels(dc, same, 2, 2, pixels));
        CHECK(memcmp(pixels, sourcePixels, sizeof(sourcePixels)) == 0);
        DeleteObject(same);
    }

    for (DWORD filter = 0; filter < STOCK_FILTERS; ++filter) {
        HBITMAP resized = CreateResampledBitmap(dc, source, 3, 3, filter);
        HBITMAP wrapped = CreateResampledBitmap(dc, source, 3, 3, filter + STOCK_FILTERS);
        CHECK(resized != NULL && wrapped != NULL);
        if (resized && wrapped) {
            CHECK(Dimensions(resized, 3, 3));
            CHECK(ReadPixels(dc, resized, 3, 3, pixels));
            CHECK(ReadPixels(dc, wrapped, 3, 3, wrappedPixels));
            CHECK(memcmp(pixels, wrappedPixels, sizeof(pixels)) == 0);
            if (filter == STOCK_FILTER_LANCZOS8) {
                CHECK(memcmp(pixels, lanczos8Pixels, sizeof(pixels)) == 0);
            } else if (filter == STOCK_FILTER_TRIANGLE) {
                CHECK(memcmp(pixels, trianglePixels, sizeof(pixels)) == 0);
            }
        }
        if (resized) { DeleteObject(resized); }
        if (wrapped) { DeleteObject(wrapped); }
    }

    HBITMAP stock = CreateResampledBitmap(dc, source, 3, 3, STOCK_FILTER_TRIANGLE);
    HBITMAP custom = CreateUserFilterResampledBitmap(dc, source, 3, 3, TriangleFilter, 1.0);
    CHECK(stock != NULL && custom != NULL);
    if (stock && custom) {
        CHECK(customCalls > 0);
        CHECK(ReadPixels(dc, stock, 3, 3, pixels));
        CHECK(ReadPixels(dc, custom, 3, 3, wrappedPixels));
        CHECK(memcmp(pixels, wrappedPixels, sizeof(pixels)) == 0);
    }
    if (stock) { DeleteObject(stock); }
    if (custom) { DeleteObject(custom); }

    HBITMAP clamped = CreateResampledBitmap(dc, source, 0, 0, STOCK_FILTER_LANCZOS8);
    CHECK(clamped != NULL);
    if (clamped) {
        CHECK(Dimensions(clamped, 1, 1));
        DeleteObject(clamped);
    }
    clamped = CreateResampledBitmap(dc, source, 4097, 1, STOCK_FILTER_LANCZOS8);
    CHECK(clamped != NULL);
    if (clamped) {
        CHECK(Dimensions(clamped, 4096, 1));
        DeleteObject(clamped);
    }

    CHECK(CreateResampledBitmap(dc, NULL, 1, 1, STOCK_FILTER_LANCZOS8) == NULL);
    CHECK(GetLastError() == E_INVALID_BITMAP);
    CHECK(CreateUserFilterResampledBitmap(dc, source, 1, 1, NULL, 1.0) == NULL);
    CHECK(GetLastError() == E_UNABLE_TO_SET_FILTER);
    CHECK(CreateUserFilterResampledBitmap(dc, source, 1, 1, TriangleFilter, -0.1) == NULL);
    CHECK(GetLastError() == E_UNABLE_TO_SET_FILTER);
    CHECK(CreateUserFilterResampledBitmap(dc, source, 1, 1, TriangleFilter, 16.1) == NULL);
    CHECK(GetLastError() == E_UNABLE_TO_SET_FILTER);
    HBITMAP monochrome = CreateBitmap(2, 2, 1, 1, NULL);
    CHECK(monochrome != NULL);
    if (monochrome) {
        CHECK(CreateResampledBitmap(dc, monochrome, 1, 1, STOCK_FILTER_LANCZOS8) == NULL);
        CHECK(GetLastError() == E_INVALID_BITMAP_DATA);
        DeleteObject(monochrome);
    }

    SelectObject(dc, oldBitmap);
    DeleteObject(source);
    DeleteObject(reference);
    DeleteDC(dc);
    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
