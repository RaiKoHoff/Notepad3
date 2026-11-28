#include <cstdlib>
#include <new>

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
