#include <cstdlib>
#include <new>
#include <string>

extern "C" {
#include "Helpers.h"
#include "PathLib.h"
DWORD InfoBoxLastError(LPCWSTR message, DWORD error);
}

#include "ResolveSelectionForOpen.inc"

extern "C" HPATHL TestResolveSelectionForOpen(int* line, bool* isDir)
{
    return _ResolveSelectionForOpen(line, isDir);
}

extern "C" bool TestExtractSelection(WCHAR* output, size_t capacity)
{
    std::wstring token;
    bool const ok = ExtractSelectionOrTokenAtCaret(token);
    return SUCCEEDED(StringCchCopyW(output, capacity, token.c_str())) && ok;
}

static bool failNextNew;

extern "C" void TestFailNextNew(void)
{
    failNextNew = true;
}

void* operator new(size_t size)
{
    if (failNextNew) {
        failNextNew = false;
        throw std::bad_alloc();
    }
    if (void* memory = std::malloc(size ? size : 1)) {
        return memory;
    }
    throw std::bad_alloc();
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, size_t) noexcept { std::free(memory); }
