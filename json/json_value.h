#pragma once

#include <json/json_reader.h>
#include <json/json_writer.h>
#include <value.h>

namespace reflpp {
namespace json {

template <typename Stream, typename T, std::enable_if_t<IsValue<T>, int>>
void ToJson(Stream& s, const T& t);

namespace _ {

// the type object and array in json allow that elements have different type
template <typename Stream>
inline void FormatJsonObject(Stream& s, const Directory& dict) {
    s.push_back('{');
    Join(s, dict.cbegin(), dict.cend(), ',', [&s](const auto& jsv) constexpr {
        FormatJsonKey(s, jsv.first);
        s.push_back(':');
        ToJson(s, jsv.second);
    });
    s.push_back('}');
}

template <typename Stream>
inline void FormatJsonArray(Stream& s, const List& list) {
    s.push_back('[');
    Join(s, list.cbegin(), list.cend(), ',',
         [&s](const auto& jsv) constexpr { ToJson(s, jsv); });
    s.push_back(']');
}

template <typename T, std::enable_if_t<IsValue<T>, int> = 0>
void ParseJsonValue(Lexer& lex, T& value) {
    auto is_floatpoint = [](std::string_view s) -> bool {
        for (const auto& ch : s) {
            if (ch == '.' || ch == 'e' || ch == 'E') {
                return true;
            }
        }
        return false;
    };

    switch (lex.token()) {
        case kTBool:
            ParseItem(lex, value.AsBool());
            break;
        case kTStr:
            ParseItem(lex, value.AsString());
            break;
        case kTNum:
            if (is_floatpoint(lex.expr()))
                ParseItem(lex, value.AsFloat());
            else
                ParseItem(lex, value.AsInt());
            break;
        case kTNull:
            value.AsNull();
            lex.Next();
            break;
        case kTLBrace:
            ParseJsonObject(lex, value.AsDirectory());
            break;
        case kTLSqBracket:
            ParseJsonArray(lex, value.AsList());
            break;
        default:
            lex.E(kErrorParseFailure, "unexpected token `{}`",
                  TokenString(lex.token()));
            break;
    }
}

void ParseJsonArray(Lexer& lex, List& lst) {
    lex.Must(kTLSqBracket);
    lex.Next();

    Value value;
    lst.Clear();
    while (!lex.IsControlToken()) {
        if (lex.token() == kTRSqBracket) {
            lex.Next();
            return;
        }

        if (lst) {
            lex.Must(kTComma);
            lex.Next();
        }

        ParseJsonValue(lex, value);
        lst.Append(std::move(value));
    }

    lex.E(kErrorUnexpectedTerminate);
}

void ParseJsonObject(Lexer& lex, Directory& dict) {
    lex.Must(kTLBrace);
    lex.Next();

    Value value;
    dict.Clear();
    while (!lex.IsControlToken()) {
        if (lex.token() == kTRBrace) {
            lex.Next();
            return;
        }

        if (dict) {
            lex.Must(kTComma);
            lex.Next();
        }

        lex.Must(kTStr);
        auto key = lex.PassExpr();

        lex.Next();
        lex.Must(kTColon);
        lex.Next();

        ParseJsonValue(lex, value);
        dict.Insert(key, std::move(value));
    }

    lex.E(kErrorUnexpectedTerminate);
}

template <typename T, std::enable_if_t<IsValue<T>, int> _ = 0>
void ParseItem(Lexer& lex, T& value) {
    auto& dict = value.AsDirectory();

    lex.Must(kTLBrace);
    lex.Next();

    while (!lex.IsControlToken()) {
        if (lex.token() == kTRBrace) {
            lex.Next();
            break;
        }

        if (dict) {
            lex.Must(kTComma);
            lex.Next();
        }

        lex.Must(kTStr);
        auto key = lex.PassExpr();

        lex.Next();
        lex.Must(kTColon);
        lex.Next();

        ParseJsonValue(lex, dict[key]);
    }
}

}  // namespace _

template <typename Stream, typename T, std::enable_if_t<IsValue<T>, int> _ = 0>
void ToJson(Stream& s, const T& t) {
    switch (t.type()) {
        case Value::Type::kNull:
            FormatJsonValue(s, nullptr);
            break;
        case Value::Type::kBoolean:
            FormatJsonValue(s, t.template As<Value::Boolean>());
            break;
        case Value::Type::kInt:
            FormatJsonValue(s, t.template As<Value::Int>());
            break;
        case Value::Type::kFloat:
            FormatJsonValue(s, t.template As<Value::Float>());
            break;
        case Value::Type::kString:
            FormatJsonValue(s, t.template As<Value::String>());
            break;
        case Value::Type::kList:
            _::FormatJsonArray(s, t.template As<Value::Array>());
            break;
        case Value::Type::kDirectory:
            _::FormatJsonObject(s, t.template As<Value::Object>());
            break;
    }
}

template <typename T, std::enable_if_t<IsValue<T>, int> _ = 0>
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
