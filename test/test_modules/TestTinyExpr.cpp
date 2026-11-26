#include "..\..\src\tinyexprcpp\np3tinyexpr.h"

#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <utility>

namespace expr = np3::tinyexpr;

static int failures;
#define CHECK(condition) do { if (!(condition)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #condition); ++failures; } } while (0)

static void CheckValue(const char* expression, double expected, bool logical)
{
    const auto result = expr::evaluate(expression);
    if (result.error != 0 || std::isnan(result.value) || std::fabs(result.value - expected) > 1e-10 || result.logical != logical) {
        std::fprintf(stderr, "expression '%s': result %.17g, error %lld, logical %d\n",
                     expression, result.value, static_cast<long long>(result.error), result.logical);
        ++failures;
    }
    const auto unclassified = expr::evaluate(expression, false);
    CHECK(unclassified.error == result.error && unclassified.value == result.value && !unclassified.logical);
}

static void CheckInvalid(const char* expression, std::intptr_t expectedError)
{
    const auto result = expr::evaluate(expression);
    CHECK(std::isnan(result.value));
    CHECK(result.error == expectedError);
    CHECK(!result.logical);
}

int main()
{
    CHECK(std::setlocale(LC_ALL, "C") != nullptr);
    CHECK(expr::is_expression_start(nullptr));
    for (int c = 0; c <= 255; ++c) {
        for (char next : { '\0', '=', '&', '|' }) {
            const char input[] = { static_cast<char>(c), next, '\0' };
            const bool expected = (c >= '0' && c <= '9') ||
                (c == 0 || (c < 128 && std::strchr("+-*/:^%!<>(),; \t\n\r", c))) ||
                c == 0xD7 || c == 0xF7 ||
                (c == '=' && next == '=') || (c == '&' && next == '&') || (c == '|' && next == '|');
            CHECK(expr::is_expression_start(input) == expected);
        }
    }
    CheckValue("=1 + 2", 3.0, false);
    CheckValue("0b101 + 0B11", 8.0, false);
    CheckValue("0b00000001", 1.0, false);
    CheckValue("1/* comment */+2", 3.0, false);
    CheckValue("1// comment\n+2", 3.0, false);
    CheckValue("log(100)", 2.0, false);
    CheckValue("(1 < 2)", 1.0, true);
    CheckValue("if(1 < 2, 5, 6)", 5.0, false);
    CheckValue("1/* > */+2", 3.0, false);
    CheckValue("0b10 + 3", 5.0, false);
    CheckInvalid(nullptr, 1);
    CheckInvalid("", 1);
    CheckInvalid("0b1111111111 + * 2", 16);
    CheckInvalid("1/**/2", 6);

    char binary[68] = "0b";
    std::memset(binary + 2, '1', 65);
    binary[67] = '\0';
    CheckValue(binary, std::ldexp(1.0, 64), false);

    CHECK(expr::is_logical("((1 < 2))"));
    CHECK(!expr::is_logical("if(1 < 2, 5, 6)"));
    CHECK(!expr::is_logical("1 << 2"));
    CHECK(!expr::is_logical("1/* > */+2"));
    CHECK(!expr::is_logical("truevalue"));
    CHECK(!expr::is_logical("\"a>b\""));
    CHECK(!expr::is_logical(nullptr));

    std::intptr_t error = -1;
    double L = 2.0, I = 3.0, N = 4.0;
    expr::Binding vars[] = { { "L", &L }, { "I", &I }, { "N", &N } };
    auto compiled = expr::Expression::compile("L+I+N+log(100)", vars, error);
    CHECK(compiled && error == 0);
    if (compiled) {
        CHECK(compiled.evaluate() == 11.0);
        L = 10.0; I = 1.0; N = 2.0;
        CHECK(compiled.evaluate() == 15.0);
        auto moved = std::move(compiled);
        CHECK(!compiled && moved && std::isnan(compiled.evaluate()));
        CHECK(moved.evaluate() == 15.0);
    }
    double logValue = 7.0;
    expr::Binding logVar[] = { { "log", &logValue } };
    compiled = expr::Expression::compile("log + 1", logVar, error);
    CHECK(compiled && error == 0 && compiled.evaluate() == 8.0);
    char variableName[] = "var0b1";
    expr::Binding binaryName[] = { { variableName, &L } };
    compiled = expr::Expression::compile("var0b1 + 1", binaryName, error);
    CHECK(compiled && error == 0);
    variableName[0] = 'x';
    CHECK(compiled.evaluate() == 11.0);
    expr::Binding duplicate[] = { { "L", &L }, { "L", &I } };
    expr::Binding invalidName[] = { { nullptr, &L } };
    expr::Binding invalidAddress[] = { { "L", nullptr } };
    compiled = expr::Expression::compile("L", duplicate, error);
    CHECK(!compiled && error == 1);
    compiled = expr::Expression::compile("L", invalidName, error);
    CHECK(!compiled && error == 1);
    compiled = expr::Expression::compile("L", invalidAddress, error);
    CHECK(!compiled && error == 1);
    compiled = expr::Expression::compile("0b1111111111 + * 2", {}, error);
    CHECK(!compiled && error == 16 && std::isnan(compiled.evaluate()));

    if (std::setlocale(LC_ALL, "German_Germany.1252") && std::localeconv()->decimal_point[0] == ',') {
        CheckValue("sum(1,5;2,25)", 3.75, false);
        CheckValue("log(100)", 2.0, false);
        compiled = expr::Expression::compile("sum(L;1,5)", vars, error);
        CHECK(compiled && error == 0 && compiled.evaluate() == 11.5);
        std::puts("Comma-decimal locale checks passed.");
    } else {
        std::puts("SKIP: comma-decimal locale unavailable.");
    }
    CHECK(std::setlocale(LC_ALL, "C") != nullptr);
    if (failures) {
        std::fprintf(stderr, "%d TinyExpr checks failed.\n", failures);
        return 1;
    }
    std::puts("TinyExpr C++ checks passed.");
    return 0;
}
