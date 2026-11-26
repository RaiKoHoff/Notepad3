#ifndef NP3_TINYEXPR_H
#define NP3_TINYEXPR_H

#include <cstdint>
#include <memory>
#include <span>

namespace np3::tinyexpr {

inline constexpr unsigned codepage = 1252U;
inline constexpr char invalid_character = '#';

inline bool is_expression_start(const char* text) noexcept
{
    if (!text || (*text >= '0' && *text <= '9')) {
        return true;
    }
    switch (*text) {
    case '\0': case '+': case '-': case '*': case -41: case '/': case ':': case -9:
    case '^': case '%': case '!': case '<': case '>': case '(': case ')': case ',':
    case ';': case ' ': case '\t': case '\n': case '\r':
        return true;
    case '=': return text[1] == '=';
    case '&': return text[1] == '&';
    case '|': return text[1] == '|';
    default: return false;
    }
}

struct Evaluation {
    double value;
    // Zero on success; otherwise a 1-based position in the original input.
    std::intptr_t error;
    bool logical;
};

// Names are copied at compile time; addresses must outlive the expression.
struct Binding {
    const char* name;
    const double* address;
};

class Expression {
public:
    Expression() noexcept;
    ~Expression();
    Expression(Expression&&) noexcept;
    Expression& operator=(Expression&&) noexcept;
    Expression(const Expression&) = delete;
    Expression& operator=(const Expression&) = delete;

    static Expression compile(const char* expression, std::span<const Binding> bindings, std::intptr_t& error) noexcept;
    explicit operator bool() const noexcept;
    double evaluate() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

Evaluation evaluate(const char* expression, bool classify = true) noexcept;
bool is_logical(const char* expression) noexcept;
bool supports_64bit() noexcept;

} // namespace np3::tinyexpr

#endif
