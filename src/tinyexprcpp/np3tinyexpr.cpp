/* encoding: UTF-8
 *
 * Notepad3 compatibility interface for TinyExpr++.
 */

#include "tinyexpr.h"  // C++ TinyExpr++ header
#include "np3tinyexpr.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <climits>
#include <clocale>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <set>
#include <string>

// ---------------------------------------------------------------------------
// Compatibility: old tinyexpr C library provided log(x) = log10(x).
// TinyExpr++ removed log(); users must use ln() or log10() explicitly.
// Register "log" as a custom function so existing expressions keep working.
// ---------------------------------------------------------------------------
static te_type te_compat_log(te_type x) { return std::log10(x); }

static void te_cif_add_compat_functions(te_parser &parser)
{
    parser.add_variable_or_function({ "log", te_compat_log });
}

// ---------------------------------------------------------------------------
// Locale-aware separator configuration.
// Queries the C runtime locale (set by Notepad3's SetMuiLocaleAll) to
// determine the decimal point character.  If the locale uses comma (','),
// configure TinyExpr++ to match: decimal = ',' , list = ';'.
// Otherwise keep the US defaults: decimal = '.' , list = ','.
// ---------------------------------------------------------------------------
static void te_cif_configure_separators(te_parser &parser)
{
    const struct lconv *lc = localeconv();
    if (lc && lc->decimal_point && lc->decimal_point[0] == ',') {
        parser.set_decimal_separator(',');
        parser.set_list_separator(';');
    }
    // else: keep defaults ('.' and ',')
}

// Common parser setup. Must run AFTER set_variables_and_functions(), which
// replaces any earlier registrations (including the compat "log").
static void te_cif_init_parser(te_parser &parser)
{
    te_cif_add_compat_functions(parser);
    te_cif_configure_separators(parser);
}

// ---------------------------------------------------------------------------
// Pre-pass over the caller's expression. The result has EXACTLY the same
// length as the input, so parse-error positions reported by TinyExpr++ index
// the caller's original text. It
//   - blanks a leading `=` (spreadsheet prefix, which TinyExpr++ would strip
//     and thereby shift positions; it is not an equality operator),
//   - blanks terminated block comments and line comments (TinyExpr++ erases
//     them, which would shift positions); an unterminated block comment is
//     left for TinyExpr++ to report,
//   - rewrites C-style binary literals `0b<bits>` (or `0B<bits>`) to their
//     decimal equivalent, padded with blanks to the original length, since
//     TinyExpr++ tokenizes via std::strtod which only accepts `0x` (hex) and
//     decimal/scientific. Hex is left alone (parser handles it). Octal is not
//     rewritten. More than 64 significant bits saturate to UINT64_MAX.
//   - copies string literals (no escapes) verbatim.
//
// Detection rule: `0b` is recognized as a binary literal only when it begins
// at a token-start position (start-of-string, after whitespace, an operator,
// `(`, `,`, `;`, etc.). Inside a variable name (e.g. `var0b1`) it is left as-is.
// ---------------------------------------------------------------------------
static std::string te_cif_preprocess(const char *expression)
{
    std::string out;
    if (!expression) {
        return out;
    }
    out.assign(expression);
    const size_t n = out.size();

    size_t i        = 0;
    bool   numStart = true; // expression start counts as a token boundary
    if (n > 0 && out[0] == '=') {
        out[i++] = ' ';
    }
    while (i < n) {
        const char c = out[i];

        if (c == '"') { // string literal
            const size_t e = out.find('"', i + 1);
            i              = (e == std::string::npos) ? n : e + 1;
            numStart       = false;
            continue;
        }
        if (c == '/' && i + 1 < n && out[i + 1] == '*') { // block comment
            const size_t e = out.find("*/", i);
            if (e == std::string::npos) {
                break; // unterminated: let TinyExpr++ report it
            }
            out.replace(i, e + 2 - i, e + 2 - i, ' ');
            i        = e + 2;
            numStart = true;
            continue;
        }
        if (c == '/' && i + 1 < n && out[i + 1] == '/') { // line comment
            size_t e = out.find_first_of("\n\r", i);
            if (e == std::string::npos) {
                e = n;
            }
            out.replace(i, e - i, e - i, ' ');
            i        = e;
            numStart = true;
            continue;
        }
        if (numStart && c == '0' && i + 2 < n && (out[i + 1] == 'b' || out[i + 1] == 'B') &&
            (out[i + 2] == '0' || out[i + 2] == '1')) {
            const size_t end = out.find_first_not_of("01", i + 2);
            const size_t stop = (end == std::string::npos) ? n : end;
            size_t       sig  = out.find('1', i + 2); // skip leading zeros
            unsigned long long value = 0;
            if (sig == std::string::npos || sig >= stop) {
                sig = stop; // all zeros
            }
            if (stop - sig > 64) {
                value = (std::numeric_limits<unsigned long long>::max)();
            } else {
                for (size_t k = sig; k < stop; ++k) {
                    value = (value << 1) | static_cast<unsigned long long>(out[k] - '0');
                }
            }
            // Decimal digits never outnumber the literal's characters.
            std::string dec = std::to_string(value);
            dec.resize(stop - i, ' ');
            out.replace(i, stop - i, dec);
            i        = stop;
            numStart = false;
            continue;
        }

        switch (static_cast<unsigned char>(c)) {
        case '+': case '-': case '*': case '/': case '%': case '^': case '~':
        case '(': case ',': case ';':
        case '<': case '>': case '=': case '!':
        case '|': case '&': case ':':
        case 0xD7: case 0xF7: // cp1252 multiply / divide signs
        case ' ': case '\t': case '\n': case '\r':
            numStart = true;
            break;
        default:
            numStart = false;
            break;
        }
        ++i;
    }
    return out;
}

// ---------------------------------------------------------------------------
// Boolean-expression detection
//
// An expression is classified as "logical" when, at parenthesis depth 0,
// it contains any of:
//   - relational / equality / logical operators:
//     `==` `=` `!=` `<>` `<=` `>=` `<` `>` `&&` `||` `!`  (unary or `!=`)
//   - the bare keywords `true` / `false`
//   - an outermost call to AND, OR, NOT, ISERR, ISERROR, ISNA, ISNAN,
//     ISEVEN, ISODD
// IF / IFS are intentionally excluded - they return arbitrary user values
// (`IF(a>b, 5, 10)` is not a boolean expression even though `a>b` is).
//
// Expects text already run through te_cif_preprocess() (comments blanked, so
// they cannot trigger classification). Bit-shift `<<`, `>>` and bit-rotate
// `<<<`, `>>>` are explicitly consumed without triggering classification.
// ---------------------------------------------------------------------------
namespace {

constexpr std::array<const char *, 9> kLogicalFunctions = {
    "and",    "or",      "not",
    "iserr",  "iserror", "isna",  "isnan",
    "iseven", "isodd",
};

inline bool te_is_ident_char(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '.';
}

inline char te_ascii_tolower(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

// Case-insensitive ASCII match of `kw` (lowercase) against expr[pos..].
// Returns the matched length (> 0) on success and only if the byte just
// past the match is NOT an identifier continuation. Returns 0 on no match.
size_t te_match_ci(const std::string &expr, size_t pos, const char *kw)
{
    size_t i = 0;
    while (kw[i]) {
        if (pos + i >= expr.size() || te_ascii_tolower(expr[pos + i]) != kw[i]) {
            return 0;
        }
        ++i;
    }
    if (pos + i < expr.size() &&
        te_is_ident_char(static_cast<unsigned char>(expr[pos + i]))) {
        return 0;
    }
    return i;
}

bool te_is_logical_keyword_at(const std::string &expr, size_t pos)
{
    return te_match_ci(expr, pos, "true") > 0 || te_match_ci(expr, pos, "false") > 0;
}

bool te_is_logical_func_at(const std::string &expr, size_t pos)
{
    for (const char *name : kLogicalFunctions) {
        size_t const consumed = te_match_ci(expr, pos, name);
        if (consumed == 0) {
            continue;
        }
        size_t after = pos + consumed;
        while (after < expr.size() && (expr[after] == ' ' || expr[after] == '\t')) {
            ++after;
        }
        if (after < expr.size() && expr[after] == '(') {
            return true;
        }
    }
    return false;
}

// Strip whitespace and any fully-enclosing outer parenthesis pairs so that
// `(1 == 1)` is classified the same as `1 == 1`.
std::string te_strip_outer_parens(std::string s)
{
    auto trim = [](std::string &t) {
        size_t a = 0;
        while (a < t.size() && std::isspace(static_cast<unsigned char>(t[a]))) ++a;
        size_t b = t.size();
        while (b > a && std::isspace(static_cast<unsigned char>(t[b - 1]))) --b;
        t.assign(t, a, b - a);
    };
    trim(s);
    while (s.size() >= 2 && s.front() == '(' && s.back() == ')') {
        // Find the close-paren that balances the leading '('. If it isn't
        // at the very end, the outer parens don't fully wrap (e.g.
        // "(a)+(b)") - or the parens are unbalanced - so stop.
        int    depth    = 0;
        size_t closePos = std::string::npos;
        bool   inStr    = false;
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '"') {
                inStr = !inStr;
            }
            else if (inStr) {
                continue;
            }
            else if (s[i] == '(') {
                ++depth;
            }
            else if (s[i] == ')' && --depth == 0) {
                closePos = i;
                break;
            }
        }
        if (closePos != s.size() - 1) {
            break;
        }
        s.assign(s, 1, s.size() - 2);
        trim(s);
    }
    return s;
}

bool te_expression_is_logical(const std::string &raw)
{
    const std::string expr = te_strip_outer_parens(raw);
    int               depth       = 0;
    bool              after_ident = false;
    const size_t      n           = expr.size();
    for (size_t i = 0; i < n; ++i) {
        const char c   = expr[i];
        const char nxt = (i + 1 < n) ? expr[i + 1] : '\0';

        if (c == '"') {
            size_t e = expr.find('"', i + 1);
            i        = (e == std::string::npos) ? n - 1 : e;
            after_ident = false;
            continue;
        }

        if (c == '(') {
            ++depth;
            after_ident = false;
            continue;
        }
        if (c == ')') {
            if (depth > 0) {
                --depth;
            }
            after_ident = false;
            continue;
        }

        if (depth == 0) {
            // `<<`, `>>` (shift) and `<<<`, `>>>` (rotate) are NOT boolean
            // producers; consume and keep scanning. Lone `<` / `>` and the
            // `<=`, `>=`, `<>` variants DO classify as logical and fall
            // through to the return below.
            if ((c == '<' || c == '>') && c == nxt) {
                ++i;
                if (i + 1 < n && expr[i + 1] == c) {
                    ++i;
                }
                after_ident = false;
                continue;
            }

            // Relational / equality / logical operators.
            if (c == '=' || c == '<' || c == '>' || c == '!') {
                return true;
            }
            if ((c == '&' && nxt == '&') || (c == '|' && nxt == '|')) {
                return true;
            }

            // Identifier start: check for boolean keyword / boolean function call.
            if (!after_ident && te_is_ident_char(static_cast<unsigned char>(c)) &&
                !(c >= '0' && c <= '9')) {
                if (te_is_logical_keyword_at(expr, i) ||
                    te_is_logical_func_at(expr, i)) {
                    return true;
                }
            }
        }

        after_ident = te_is_ident_char(static_cast<unsigned char>(c));
    }
    return false;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Helper: map TinyExpr++ error state to old 1-based error position
// Old convention: 0 = success, >= 1 = 1-based error position
// ---------------------------------------------------------------------------
static std::intptr_t map_error(const te_parser &parser, const size_t exprLen)
{
    if (parser.success()) {
        return 0;
    }
    int64_t pos = parser.get_last_error_position();
    if (pos < 0) {
        return 1; // unknown error position — report position 1
    }
    // convert 0-based to 1-based; never point past the end of the input
    return static_cast<std::intptr_t>((std::min)(static_cast<size_t>(pos) + 1, (std::max)(exprLen, size_t{ 1 })));
}

namespace np3::tinyexpr {

struct Expression::Impl {
    te_parser parser;
};

Expression::Expression() noexcept = default;
Expression::~Expression() = default;
Expression::Expression(Expression&&) noexcept = default;
Expression& Expression::operator=(Expression&&) noexcept = default;

Expression::operator bool() const noexcept
{
    return impl_ != nullptr;
}

Expression Expression::compile(const char* expression, std::span<const Binding> bindings, std::intptr_t& error) noexcept
{
    error = 1;
    Expression result;
    if (!expression || !*expression) {
        return result;
    }
    try {
        auto ctx = std::make_unique<Impl>();
        if (!bindings.empty()) {
            std::set<te_variable> cppVars;
            for (const Binding& binding : bindings) {
                if (!binding.name || !binding.address ||
                    !cppVars.insert({ std::string(binding.name), binding.address }).second) {
                    return result;
                }
            }
            ctx->parser.set_variables_and_functions(std::move(cppVars));
        }
        te_cif_init_parser(ctx->parser);
        (void)ctx->parser.evaluate(te_cif_preprocess(expression));
        if (!ctx->parser.success()) {
            error = map_error(ctx->parser, std::strlen(expression));
            return result;
        }
        error = 0;
        result.impl_ = std::move(ctx);
    }
    catch (...) {
    }
    return result;
}

double Expression::evaluate() const noexcept
{
    if (!impl_) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    try {
        return impl_->parser.evaluate();
    }
    catch (...) {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

Evaluation evaluate(const char* expression, bool classify) noexcept
{
    Evaluation result{ std::numeric_limits<double>::quiet_NaN(), 1, false };
    if (!expression || !*expression) {
        return result;
    }
    try {
        te_parser parser;
        te_cif_init_parser(parser);
        std::string const rewritten = te_cif_preprocess(expression);
        result.value = parser.evaluate(rewritten);
        result.error = map_error(parser, rewritten.size());
        if (classify && parser.success()) {
            result.logical = te_expression_is_logical(rewritten);
        }
    }
    catch (...) {
        return { std::numeric_limits<double>::quiet_NaN(), 1, false };
    }
    return result;
}

bool is_logical(const char* expression) noexcept
{
    if (!expression || !*expression) {
        return false;
    }
    try {
        return te_expression_is_logical(te_cif_preprocess(expression));
    }
    catch (...) {
        return false;
    }
}

bool supports_64bit() noexcept
{
    return te_parser::supports_64bit();
}

} // namespace np3::tinyexpr
