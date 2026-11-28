#pragma once
#include <windows.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <strsafe.h>

#ifdef __cplusplus
extern "C" {
#endif
int te_supports_64bit(void);
void TestSetTinyExprMode(int mode);
void TinyExprToStringA(LPSTR dest, size_t size, double value, bool logical);
void TinyExprToString(LPWSTR dest, size_t size, double value, bool logical);
#ifdef __cplusplus
}
namespace np3::tinyexpr {
inline bool supports_64bit() noexcept { return te_supports_64bit() != 0; }
}
#endif
