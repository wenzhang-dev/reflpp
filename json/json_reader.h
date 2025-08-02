#pragma once

#include <field_name.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <json/ec.h>
#include <json/utf8.h>
#include <type_trait.h>
#include <utils.h>

#include <charconv>
#include <iostream>
#include <system_error>

#define JSON_TOKEN_LIST(__) \
    __(kTColon, ":")        \
    __(kTComma, ",")        \
    __(kTLSqBracket, "[")   \
    __(kTRSqBracket, "]")   \
    __(kTLBrace, "{")       \
    __(kTRBrace, "}")       \
    __(kTStr, "str")        \
    __(kTNum, "num")        \
    __(kTBool, "bool")      \
    __(kTNull, "null")      \
    __(kTEof, "<eof>")      \
    __(kTError, "<error>")

namespace reflpp {
namespace json {

namespace _ {

// clang-format off
enum Token : std::uint8_t {
#define __(A, B) A,
        JSON_TOKEN_LIST(__)
#undef __
};

static const char* TokenString(Token tk) {
    switch (tk) {
#define __(A, B) case A: return B;
    JSON_TOKEN_LIST(__)
#undef __
    }
    return "unknown token";
}
// clang-format on

struct Lexer {
    Lexer(std::string_view data) : data_(data) {
        // let position point to the first token
        Next();
    }

    Lexer(Lexer&&) = default;
    Lexer& operator=(Lexer&&) = default;

    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;

    inline Token Next();
    inline bool Must(Token tk);
    Token token() { return token_; }
    std::string_view expr() { return expr_; }

    Token E(int ec) {
        ec_ = make_error(ec);
        detail_emsg_ = error_category.message(ec);
        return (token_ = kTError);
    }

    template <typename... Args>
    Token E(int ec, const char* fmt, const Args&... args) {
        ec_ = make_error(ec);
        detail_emsg_ = fmt::vformat(fmt, fmt::make_format_args(args...));
        return (token_ = kTError);
    }

    std::string PassExpr() { return std::move(expr_); }
    Token Ret(Token tk, const char* expr) {
        return Ret(tk, std::string_view(expr));
    }
    Token Ret(Token tk, std::string_view expr) {
        expr_ = expr;
        return (token_ = tk);
    }
    Token Ret(Token tk, std::string&& expr) {
        expr_ = std::move(expr);
        return (token_ = tk);
    }

    template <std::size_t N>
    bool Literal(const char (&arr)[N]) {
        // Notes, `N - 1` equals to string length
        std::string_view src(data_.begin() + cursor_, N - 1);
        std::string_view dst(arr, N - 1);
        if (src == dst) {
            cursor_ += (N - 1);
            return true;
        }
        return false;
    }

    inline Token LexStr();
    inline Token LexNum();
    inline Token LexBoolOrNull();

    bool IsEof() const { return token_ == kTEof; }
    bool IsError() const { return static_cast<bool>(ec_); }
    bool IsControlToken() const { return IsEof() || IsError(); }

    std::error_code error() { return ec_; }
    std::string_view detail_error() const { return detail_emsg_; }

    inline bool NextChar();
    inline std::optional<std::uint32_t> PeekChar();

    std::error_code ec_;
    std::string detail_emsg_;

    Token token_;
    std::uint32_t c0_{0};
    std::uint32_t c0_length_{0};
    std::size_t cursor_{0};
    std::string expr_;
    std::string_view data_;
};

template <typename T, std::enable_if_t<IsAggregateStruct<T>, int> _ = 0>
void ParseItem(Lexer& lex, T& value) {
    auto field_names = ::reflpp::kFieldNames<T>;
    std::unordered_map<std::string_view, std::size_t> mapping;
    for (std::size_t i = 0; i < field_names.size(); i++) {
        mapping.insert(std::make_pair(field_names[i], i));
    }

    lex.Must(kTLBrace);
    lex.Next();

    bool started = false;
    while (!lex.IsControlToken()) {
        // handle empty struct first
        if (lex.token() == kTRBrace) {
            lex.Next();
            break;
        }

        if (started) {
            lex.Must(kTComma);
            lex.Next();
        }

        lex.Must(kTStr);
        auto key = lex.PassExpr();

        lex.Next();
        lex.Must(kTColon);
        lex.Next();

        // for compatibility, here ignore unknown fields in json
        // FIXME: optimize the performance
        if (auto itr = mapping.find(key); itr != mapping.end()) {
            ForEach(value, [&itr, &lex](auto idx, auto& v) {
                if (itr->second != idx) return;
                ParseItem(lex, v);
            });
        }
        started = true;
    }
}

template <typename T, std::enable_if_t<IsBool<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    if (lex.Must(kTBool)) {
        value = lex.expr() == "true";
        lex.Next();
    }
}

template <typename T, std::enable_if_t<IsNumeric<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    if (lex.Must(kTNum)) {
        if (LexicalCast(lex.expr(), value)) {
            lex.E(kErrorMismatchType, "expect `num` but got `{}`", lex.expr());
            return;
        }

        lex.Next();
    }
}

template <typename T, std::enable_if_t<IsChar<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    if (lex.Must(kTStr)) {
        auto expr = lex.expr();
        std::uint32_t codepoint = 0;
        if (auto opt =
                Utf8DfaDecoder::Decode(expr.data(), expr.size(), &codepoint);
            opt) {
            if (opt.value() == expr.size()) {
                value = codepoint;
                lex.Next();
                return;
            }
        }
        lex.E(kErrorMismatchType, "invalid char");
    }
}

template <typename T, std::enable_if_t<IsString<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    if (lex.Must(kTStr)) {
        if constexpr (std::is_same_v<std::string, T>) {
            value = std::move(lex.PassExpr());
        } else {
            value.clear();
            auto expr = lex.expr();
            value.reserve(expr.size());
            for (const auto& ch : expr) {
                value.push_back(ch);
            }
        }
        lex.Next();
    }
}

template <typename T, std::enable_if_t<IsMapContainer<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    using U = std::remove_reference_t<T>;
    using KeyType = typename U::key_type;

    value.clear();
    lex.Must(kTLBrace);
    lex.Next();
    while (!lex.IsControlToken()) {
        if (lex.token() == kTRBrace) {
            lex.Next();
            return;
        }

        if (!value.empty()) {
            lex.Must(kTComma);
            lex.Next();
        }

        lex.Must(kTStr);
        auto key = lex.PassExpr();

        lex.Next();
        lex.Must(kTColon);
        lex.Next();

        if constexpr (IsStringLike<KeyType> || IsNumeric<KeyType> ||
                      IsChar<KeyType>) {
            ParseItem(lex, value[KeyType(key)]);
        } else {
            lex.E(kErrorMismatchType, "unsupport key type: {}", lex.expr());
        }
    }

    lex.E(kErrorUnexpectedTerminate);
}

template <typename T, std::enable_if_t<IsSequenceContainer<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    lex.Must(kTLSqBracket);
    lex.Next();

    value.clear();
    while (!lex.IsControlToken()) {
        if (lex.token() == kTRSqBracket) {
            lex.Next();
            return;
        }

        if (!value.empty()) {
            lex.Must(kTComma);
            lex.Next();
        }

        ParseItem(lex, value.emplace_back());
    }

    lex.E(kErrorUnexpectedTerminate);
}

template <typename T, std::enable_if_t<IsSetContainer<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    using U = std::remove_reference_t<T>;
    using KeyType = typename U::key_type;

    lex.Must(kTLSqBracket);
    lex.Next();

    KeyType v{};
    value.clear();
    while (!lex.IsControlToken()) {
        if (lex.token() == kTRSqBracket) {
            lex.Next();
            return;
        }

        if (!value.empty()) {
            lex.Must(kTComma);
            lex.Next();
        }

        ParseItem(lex, v);
        value.insert(std::move(v));
    }

    lex.E(kErrorUnexpectedTerminate);
}

template <typename T, std::enable_if_t<IsOptional<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    using U = std::remove_reference_t<T>;
    using ValueType = typename U::value_type;

    if (lex.token() == kTNull) {
        value = std::nullopt;
        lex.Next();
    } else {
        ValueType v{};
        ParseItem(lex, v);
        value = std::move(v);
    }
}

template <typename T, std::enable_if_t<IsSmartPtr<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    using U = std::remove_reference_t<T>;
    using ValueType = typename U::element_type;

    if (lex.token() == kTNull) {
        value = nullptr;
        lex.Next();
    } else {
        if constexpr (IsUniquePtr<T>) {
            value = std::make_unique<ValueType>();
        } else {
            value = std::make_shared<ValueType>();
        }
        ParseItem(lex, *value);
    }
}

template <typename T, std::enable_if_t<IsTuple<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    lex.Must(kTLSqBracket);
    lex.Next();

    ForEach(value, [&lex](auto i, auto& v) {
        if (lex.token() == kTRSqBracket) {
            return;
        }

        if (i != 0) {
            lex.Must(kTComma);
            lex.Next();
        }

        ParseItem(lex, v);
    });

    lex.Must(kTRSqBracket);
    lex.Next();
}

template <typename T, std::enable_if_t<IsCharArray<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    using U = std::remove_reference_t<T>;
    constexpr std::size_t n =
        sizeof(U) / sizeof(decltype(std::declval<U>()[0]));

    if (lex.token() == kTStr) {
        auto str = lex.expr();
        if (str.size() > n) {
            lex.E(kErrorArrayOutOfRange);
        } else {
            std::size_t i = 0;
            for (const auto& ch : str) {
                value[i++] = ch;
            }
            lex.Next();
        }
        return;
    }

    ParseArray(lex, value);
}

template <typename T, std::enable_if_t<IsNonCharArray<T>, int> = 0>
void ParseItem(Lexer& lex, T& value) {
    ParseArray(lex, value);
}

template <typename T, std::enable_if_t<IsFixedArray<T>, int> = 0>
void ParseArray(Lexer& lex, T& value) {
    using U = std::remove_reference_t<T>;
    constexpr std::size_t n =
        sizeof(U) / sizeof(decltype(std::declval<U>()[0]));

    lex.Must(kTLSqBracket);
    lex.Next();

    auto itr = std::begin(value);
    for (std::size_t i = 0; !lex.IsControlToken(); ++i) {
        if (lex.token() == kTRSqBracket) {
            lex.Next();
            return;
        }

        if (i >= n) break;

        if (i > 0) {
            lex.Must(kTComma);
            lex.Next();
        }

        ParseItem(lex, *itr++);
    }

    lex.E(kErrorUnexpectedTerminate);
}

inline bool Lexer::Must(Token tk) {
    if (tk != token()) {
        E(kErrorParseFailure, "expect `{}` but got `{}`", TokenString(tk),
          TokenString(token()));
    }
    return tk == token();
}

inline auto Lexer::Next() -> Token {
    if (IsError()) return token();

    while (cursor_ < data_.size()) {
        if (!NextChar()) {
            return E(kErrorInvalidUtf8Char);
        }

        switch (c0_) {
            case '[':
                return Ret(kTLSqBracket, "[");
            case ']':
                return Ret(kTRSqBracket, "]");
            case '{':
                return Ret(kTLBrace, "{");
            case '}':
                return Ret(kTRBrace, "}");
            case ',':
                return Ret(kTComma, ",");
            case ':':
                return Ret(kTColon, ":");
            case '"':
                return LexStr();
            case ' ':
            case '\b':
            case '\v':
            case '\r':
            case '\t':
            case '\n':
                continue;
            case '.':
            case 'e':
            case 'E':
            case '0' ... '9':
                return LexNum();
            default:
                return LexBoolOrNull();
        }
    }

    return Ret(kTEof, "");
}

inline auto Lexer::LexStr() -> Token {
    std::string buf;
    auto parse_escape = [&](std::uint32_t codepoint) -> bool {
        switch (codepoint) {
            case 'v':
                buf.push_back('\v');
                break;
            case 't':
                buf.push_back('\t');
                break;
            case 'r':
                buf.push_back('\r');
                break;
            case 'n':
                buf.push_back('\n');
                break;
            case '\\':
                buf.push_back('\\');
                break;
            case '"':
                buf.push_back('"');
                break;
            case '\'':
                buf.push_back('\'');
                break;
            default:
                return false;
        }
        return true;
    };
    while (cursor_ < data_.size()) {
        if (!NextChar()) {
            return E(kErrorInvalidUtf8Char);
        }

        if (c0_ == '"') {
            break;
        } else if (c0_ == '\\') {
            if (!NextChar()) {
                return E(kErrorInvalidUtf8Char);
            }

            if (!parse_escape(c0_)) {
                return E(kErrorParseFailure, "unknown escape `{}`", c0_);
            }
        } else {
            Utf8Encode(buf, c0_);
        }
    }

    if (c0_ != '"') {
        return E(kErrorParseFailure, "early terminate in string literal");
    }

    return Ret(kTStr, std::move(buf));
}

inline auto Lexer::LexNum() -> Token {
    std::string buf;
    buf.push_back(c0_);

    auto is_numeric = [](std::uint32_t codepoint) -> bool {
        return (codepoint >= '0' && codepoint <= '9') || codepoint == 'E' ||
               codepoint == 'e' || codepoint == '.';
    };

    while (cursor_ < data_.size()) {
        if (auto opt = PeekChar(); !opt) {
            return E(kErrorInvalidUtf8Char);
        } else if (is_numeric(opt.value())) {
            buf.push_back(opt.value());
            NextChar();
        } else {
            break;
        }
    }

    return Ret(kTNum, std::move(buf));
}

inline auto Lexer::LexBoolOrNull() -> Token {
    switch (c0_) {
        case 'n':
            if (Literal("ull")) return Ret(kTNull, "null");
            break;
        case 'f':
            if (Literal("alse")) return Ret(kTBool, "false");
            break;
        case 't':
            if (Literal("rue")) return Ret(kTBool, "true");
            break;
        default:
            break;
    }
    return E(kErrorParseFailure, "unknown chars: {}", expr());
}

inline bool Lexer::NextChar() {
    std::size_t remain_size = data_.size() - cursor_;
    auto opt = Utf8DfaDecoder::Decode(&data_[cursor_], remain_size, &c0_);
    if (opt) {
        c0_length_ = opt.value();
        cursor_ += c0_length_;
    }
    return opt.has_value();
}

inline std::optional<std::uint32_t> Lexer::PeekChar() {
    std::uint32_t res;
    std::size_t remain_size = data_.size() - cursor_;
    auto opt = Utf8DfaDecoder::Decode(&data_[cursor_], remain_size, &res);
    if (opt) {
        return res;
    }
    return std::nullopt;
}

}  // namespace _

template <typename T, std::enable_if_t<IsAggregateStruct<T>, int> _ = 0>
std::error_code FromJson(std::string_view json_str, T& value,
                         std::string* detail_emsg = nullptr) {
    _::Lexer lex(json_str);

    _::ParseItem(lex, value);
    lex.Must(_::kTEof);

    if (detail_emsg && lex.IsError()) {
        *detail_emsg = lex.detail_error();
    }

    return lex.error();
}

}  // namespace json
}  // namespace reflpp
