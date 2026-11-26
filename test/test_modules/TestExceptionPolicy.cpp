#include <windows.h>
#include <cstdio>

#ifdef TEST_EXCEPTION_THROWER
extern "C" __declspec(noinline) void ThrowThroughC()
{
    throw 42;
}
#else
extern "C" void ThrowThroughC();

static int destroyed;
static bool caughtStructured;
struct Guard {
    ~Guard() { ++destroyed; }
};

static bool TestCppUnwind()
{
    try {
        Guard guard;
        ThrowThroughC();
    } catch (int value) {
        return value == 42 && destroyed == 1;
    }
    return false;
}

static void RaiseStructured()
{
    try {
        RaiseException(0xE0000001, 0, 0, nullptr);
    } catch (...) {
        caughtStructured = true;
    }
}

static bool TestStructuredGuard()
{
    bool finalized = false;
    bool handled = false;
    __try {
        __try {
            RaiseStructured();
        } __finally {
            finalized = true;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        handled = true;
    }
    return handled && finalized && !caughtStructured;
}

int main()
{
    if (!TestCppUnwind() || !TestStructuredGuard()) {
        std::fputs("Exception policy checks failed.\n", stderr);
        return 1;
    }
    std::puts("Exception policy checks passed.");
    return 0;
}
#endif
