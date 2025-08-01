#pragma once

#include <field_name.h>
#include <fmt/core.h>
#include <fmt/format.h>
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

#define JSON_ERROR_LIST(__)                               \
    __(kOk, "OK")                                         \
    __(kErrorUnexpectedTerminate, "Unexpected terminate") \
    __(kErrorParseFailure, "Parse failure")               \
    __(kErrorMismatchType, "Mismatch type")               \
    __(kErrorArrayOutOfRange, "Array out of range")       \
    __(kErrorInvalidUtf8Char, "Invalid utf8 char")

namespace reflpp {
namespace json {

// clang-format off
enum ErrorCode {
#define __(A, B) A,
    JSON_ERROR_LIST(__)
#undef __
};
// clang-format on

struct JsonErrorCategory : public std::error_category {
    const char* name() const noexcept override { return "json error"; }

    // clang-format off
    std::string message(int ec) const override {
        switch (ec) {
#define __(A, B) case A: return B;
        JSON_ERROR_LIST(__)
#undef __
        }
        return "unknown error code";
    }
    // clang-format on
};

inline const JsonErrorCategory error_category;

std::error_code make_error(int ec) { return {ec, error_category}; }

namespace _ {

class JsonParser {
   public:
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
        Lexer(std::string_view data) : data_(data) {}
        Lexer(Lexer&&) = default;
        Lexer& operator=(Lexer&&) = default;

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

   public:
    JsonParser(std::string_view data) : lex_(data) {
        // let position point to the first token
        lex_.Next();
    }

    JsonParser(JsonParser&&) = default;
    JsonParser& operator=(JsonParser&&) = default;

   public:
    std::string_view detail_emsg() const { return lex_.detail_error(); }

    template <typename T, std::enable_if_t<IsAggregateStruct<T>, int> _ = 0>
    std::error_code Parse(T& value) {
        ParseItem(value);

        lex_.Must(kTEof);
        return lex_.error();
    }

   private:
    template <typename T, std::enable_if_t<IsAggregateStruct<T>, int> _ = 0>
    void ParseItem(T& value) {
        auto field_names = ::reflpp::kFieldNames<T>;
        std::unordered_map<std::string_view, std::size_t> mapping;
        for (std::size_t i = 0; i < field_names.size(); i++) {
            mapping.insert(std::make_pair(field_names[i], i));
        }

        lex_.Must(kTLBrace);
        lex_.Next();

        bool started = false;
        while (!lex_.IsControlToken()) {
            // handle empty struct first
            if (lex_.token() == kTRBrace) {
                lex_.Next();
                break;
            }

            if (started) {
                lex_.Must(kTComma);
                lex_.Next();
            }

            lex_.Must(kTStr);
            auto key = lex_.PassExpr();

            lex_.Next();
            lex_.Must(kTColon);
            lex_.Next();

            // for compatibility, here ignore unknown fields in json
            // FIXME: optimize the performance
            if (auto itr = mapping.find(key); itr != mapping.end()) {
                ForEach(value, [this, &itr](auto idx, auto& v) {
                    if (itr->second != idx) return;
                    ParseItem(v);
                });
            }
            started = true;
        }
    }

    template <typename T, std::enable_if_t<IsBool<T>, int> = 0>
    void ParseItem(T& value) {
        if (lex_.Must(kTBool)) {
            value = lex_.expr() == "true";
            lex_.Next();
        }
    }

    template <typename T, std::enable_if_t<IsNumeric<T>, int> = 0>
    void ParseItem(T& value) {
        if (lex_.Must(kTNum)) {
            if (LexicalCast(lex_.expr(), value)) {
                lex_.E(kErrorMismatchType, "expect `num` but got `{}`",
                       lex_.expr());
                return;
            }

            lex_.Next();
        }
    }

    template <typename T, std::enable_if_t<IsChar<T>, int> = 0>
    void ParseItem(T& value) {
        if (lex_.Must(kTStr)) {
            auto expr = lex_.expr();
            std::uint32_t codepoint = 0;
            if (auto opt = Utf8DfaDecoder::Decode(expr.data(), expr.size(),
                                                  &codepoint);
                opt) {
                if (opt.value() == expr.size()) {
                    value = codepoint;
                    lex_.Next();
                    return;
                }
            }
            lex_.E(kErrorMismatchType, "invalid char");
        }
    }

    template <typename T, std::enable_if_t<IsString<T>, int> = 0>
    void ParseItem(T& value) {
        if (lex_.Must(kTStr)) {
            if constexpr (std::is_same_v<std::string, T>) {
                value = std::move(lex_.PassExpr());
            } else {
                value.clear();
                auto expr = lex_.expr();
                value.reserve(expr.size());
                for (const auto& ch : expr) {
                    value.push_back(ch);
                }
            }
            lex_.Next();
        }
    }

    template <typename T, std::enable_if_t<IsMapContainer<T>, int> = 0>
    void ParseItem(T& value) {
        using U = std::remove_reference_t<T>;
        using KeyType = typename U::key_type;

        value.clear();
        lex_.Must(kTLBrace);
        lex_.Next();
        while (!lex_.IsControlToken()) {
            if (lex_.token() == kTRBrace) {
                lex_.Next();
                return;
            }

            if (!value.empty()) {
                lex_.Must(kTComma);
                lex_.Next();
            }

            lex_.Must(kTStr);
            auto key = lex_.PassExpr();

            lex_.Next();
            lex_.Must(kTColon);
            lex_.Next();

            if constexpr (IsStringLike<KeyType> || IsNumeric<KeyType> ||
                          IsChar<KeyType>) {
                ParseItem(value[KeyType(key)]);
            } else {
                lex_.E(kErrorMismatchType, "unsupport key type: {}",
                       lex_.expr());
            }
        }

        lex_.E(kErrorUnexpectedTerminate);
    }

    template <typename T, std::enable_if_t<IsSequenceContainer<T>, int> = 0>
    void ParseItem(T& value) {
        lex_.Must(kTLSqBracket);
        lex_.Next();

        value.clear();
        while (!lex_.IsControlToken()) {
            if (lex_.token() == kTRSqBracket) {
                lex_.Next();
                return;
            }

            if (!value.empty()) {
                lex_.Must(kTComma);
                lex_.Next();
            }

            ParseItem(value.emplace_back());
        }

        lex_.E(kErrorUnexpectedTerminate);
    }

    template <typename T, std::enable_if_t<IsSetContainer<T>, int> = 0>
    void ParseItem(T& value) {
        using U = std::remove_reference_t<T>;
        using KeyType = typename U::key_type;

        lex_.Must(kTLSqBracket);
        lex_.Next();

        KeyType v{};
        value.clear();
        while (!lex_.IsControlToken()) {
            if (lex_.token() == kTRSqBracket) {
                lex_.Next();
                return;
            }

            if (!value.empty()) {
                lex_.Must(kTComma);
                lex_.Next();
            }

            ParseItem(v);
            value.insert(std::move(v));
        }

        lex_.E(kErrorUnexpectedTerminate);
    }

    template <typename T, std::enable_if_t<IsOptional<T>, int> = 0>
    void ParseItem(T& value) {
        using U = std::remove_reference_t<T>;
        using ValueType = typename U::value_type;

        if (lex_.token() == kTNull) {
            value = std::nullopt;
            lex_.Next();
        } else {
            ValueType v{};
            ParseItem(v);
            value = std::move(v);
        }
    }

    template <typename T, std::enable_if_t<IsSmartPtr<T>, int> = 0>
    void ParseItem(T& value) {
        using U = std::remove_reference_t<T>;
        using ValueType = typename U::element_type;

        if (lex_.token() == kTNull) {
            value = nullptr;
            lex_.Next();
        } else {
            if constexpr (IsUniquePtr<T>) {
                value = std::make_unique<ValueType>();
            } else {
                value = std::make_shared<ValueType>();
            }
            ParseItem(*value);
        }
    }

    template <typename T, std::enable_if_t<IsTuple<T>, int> = 0>
    void ParseItem(T& value) {
        lex_.Must(kTLSqBracket);
        lex_.Next();

        ForEach(value, [&, this](auto i, auto& v) {
            if (lex_.token() == kTRSqBracket) {
                return;
            }

            if (i != 0) {
                lex_.Must(kTComma);
                lex_.Next();
            }

            ParseItem(v);
        });

        lex_.Must(kTRSqBracket);
        lex_.Next();
    }

    template <typename T, std::enable_if_t<IsCharArray<T>, int> = 0>
    void ParseItem(T& value) {
        using U = std::remove_reference_t<T>;
        constexpr std::size_t n =
            sizeof(U) / sizeof(decltype(std::declval<U>()[0]));

        if (lex_.token() == kTStr) {
            auto str = lex_.expr();
            if (str.size() > n) {
                lex_.E(kErrorArrayOutOfRange);
            } else {
                std::size_t i = 0;
                for (const auto& ch : str) {
                    value[i++] = ch;
                }
                lex_.Next();
            }
            return;
        }

        ParseArray(value);
    }

    template <typename T, std::enable_if_t<IsNonCharArray<T>, int> = 0>
    void ParseItem(T& value) {
        ParseArray(value);
    }

    template <typename T, std::enable_if_t<IsFixedArray<T>, int> = 0>
    void ParseArray(T& value) {
        using U = std::remove_reference_t<T>;
        constexpr std::size_t n =
            sizeof(U) / sizeof(decltype(std::declval<U>()[0]));

        lex_.Must(kTLSqBracket);
        lex_.Next();

        auto itr = std::begin(value);
        for (std::size_t i = 0; !lex_.IsControlToken(); ++i) {
            if (lex_.token() == kTRSqBracket) {
                lex_.Next();
                return;
            }

            if (i >= n) break;

            if (i > 0) {
                lex_.Must(kTComma);
                lex_.Next();
            }

            ParseItem(*itr++);
        }

        lex_.E(kErrorUnexpectedTerminate);
    }

    Lexer lex_;
};

inline bool JsonParser::Lexer::Must(Token tk) {
    if (tk != token()) {
        E(kErrorParseFailure, "expect `{}` but got `{}`", TokenString(tk),
          TokenString(token()));
    }
    return tk == token();
}

inline auto JsonParser::Lexer::Next() -> Token {
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

inline auto JsonParser::Lexer::LexStr() -> Token {
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

inline auto JsonParser::Lexer::LexNum() -> Token {
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

inline auto JsonParser::Lexer::LexBoolOrNull() -> Token {
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

inline bool JsonParser::Lexer::NextChar() {
    std::size_t remain_size = data_.size() - cursor_;
    auto opt = Utf8DfaDecoder::Decode(&data_[cursor_], remain_size, &c0_);
    if (opt) {
        c0_length_ = opt.value();
        cursor_ += c0_length_;
    }
    return opt.has_value();
}

inline std::optional<std::uint32_t> JsonParser::Lexer::PeekChar() {
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
    _::JsonParser parser(json_str);
    auto ec = parser.Parse(value);
    if (ec && detail_emsg) {
        *detail_emsg = parser.detail_emsg();
    }

    return ec;
}

}  // namespace json
}  // namespace reflpp
