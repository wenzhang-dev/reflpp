#include <field_name.h>
#include <fields_count.h>
#include <value.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <for_each.h>
#include <json/json_reader.h>
#include <json/json_writer.h>
#include <json/json_value.h>
#include <json/pretty_formatter.h>

#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

template <typename T>
struct fmt::formatter<std::unique_ptr<T>> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::unique_ptr<T>& ptr, FormatContext& ctx) const {
        if (ptr)
            return fmt::format_to(ctx.out(), "{}", *ptr);
        else
            return fmt::format_to(ctx.out(), "null");
    }
};

template <typename T>
struct fmt::formatter<std::shared_ptr<T>> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::shared_ptr<T>& ptr, FormatContext& ctx) const {
        if (ptr)
            return fmt::format_to(ctx.out(), "{}", *ptr);
        else
            return fmt::format_to(ctx.out(), "null");
    }
};

struct Foo {
    int a;
    bool b;
    double c;

    std::string d;
    std::vector<std::string> e;
    std::array<int, 3> f;

    std::map<std::string, std::string> g;
    std::set<int> h;

    std::optional<int> i;
    std::unique_ptr<std::string> j;
};

struct Dummy {};

struct Bar {
    Foo foo;

    int a;
    bool b;
    double c;
};

int main() {
    std::cout << "fmt version: " << FMT_VERSION / 10000 << '.'
              << (FMT_VERSION / 100 % 100) << '.' << (FMT_VERSION % 100)
              << '\n';
    std::cout << ::reflpp::FieldsCount<Foo>() << std::endl;
    std::cout << ::reflpp::FieldsCount<Dummy>() << std::endl;
    std::cout << ::reflpp::FieldsCount<Bar>() << std::endl;

    // foo to json
    Foo foo{1,
            false,
            3.14,
            "456",
            {"789", "123", "456"},
            {7, 8, 9},
            {std::make_pair("1", "2"), std::make_pair("3", "4"),
             std::make_pair("5", "6")},
            {7, 8, 9},
            1,
            std::make_unique<std::string>("234")};

    ::reflpp::ForEach(foo, [](auto idx, const auto& v) {
        std::cout << fmt::format("idx: {}, v: {}", idx, v) << std::endl;
    });

    std::cout << ::reflpp::GetFieldName<Foo, 0>() << std::endl;
    std::cout << ::reflpp::GetFieldName<Foo, 1>() << std::endl;
    std::cout << ::reflpp::GetFieldName<Foo, 2>() << std::endl;

    ::reflpp::json::PrettyJsonFormatter stream;
    ::reflpp::json::ToJson(stream, foo);

    std::cout << stream << std::endl;

    // dummy to json
    Dummy dummy;
    ::reflpp::json::PrettyJsonFormatter stream1;
    ::reflpp::json::ToJson(stream1, dummy);

    std::cout << stream1 << std::endl;

    // bar to json
    Bar bar{
        {},
        1,
        true,
        1.23
    };
    ::reflpp::json::PrettyJsonFormatter stream2;
    ::reflpp::json::ToJson(stream2, bar);

    std::cout << stream2 << std::endl;

    // json to foo
    std::string json_str = R"""(
{
    "a": 1,
    "b": false,
    "c": 3.14,
    "d": "456",
    "e": [
        "789",
        "123",
        "456"
    ],
    "f": [
        7,
        8,
        9
    ],
    "g": {
        "1": "2",
        "3": "4",
        "5": "6"
    },
    "h": [
        7,
        8,
        9
    ],
    "i": 1,
    "j": "234"
}
    )""";

    Foo foo1;
    std::string detail_emsg;
    auto ec = ::reflpp::json::FromJson(json_str, foo1, &detail_emsg);
    std::cout << ec.value() << std::endl;

    std::cout << ::reflpp::json::error_category.message(ec.value())
              << std::endl;
    std::cout << "err: " << detail_emsg << std::endl;

    ::reflpp::ForEach(foo1, [](auto idx, const auto& v) {
        std::cout << fmt::format("idx: {}, v: {}", idx, v) << std::endl;
    });

    // json to dummy
    Dummy dummy1;
    ec = ::reflpp::json::FromJson("{}", dummy1, &detail_emsg);
    std::cout << "err: " << detail_emsg << std::endl;

    // json to bar
    json_str = R"""(
{
    "foo": {
        "a": 1,
        "b": false,
        "c": 3.14,
        "d": "456",
        "e": [
            "789",
            "123",
            "456"
        ],
        "f": [
            7,
            8,
            9
        ],
        "g": {
            "1": "2",
            "3": "4",
            "5": "6"
        },
        "h": [
            7,
            8,
            9
        ],
        "i": 1,
        "j": "234"
    },
    "a": 1,
    "b": true,
    "c": 1.23
}
    )""";

    Bar bar1;
    ec = ::reflpp::json::FromJson(json_str, bar1, &detail_emsg);
    std::cout << "err: " << detail_emsg << std::endl;

    ::reflpp::ForEach(bar1.foo, [](auto idx, const auto& v) {
        std::cout << fmt::format("idx: {}, v: {}", idx, v) << std::endl;
    });

    std::cout << bar1.a << std::endl;
    std::cout << bar1.b << std::endl;
    std::cout << bar1.c << std::endl;

    // json value
    std::cout << "Json value" << std::endl;

    ::reflpp::Value v;
    auto& dict = v.AsDirectory().Insert("123", 3.14).Insert("456", false).Insert("789", 111);

    for (auto& kv : dict) {
        std::cout << "key: " << kv.first << std::endl;
    }

    ::reflpp::json::PrettyJsonFormatter stream3;
    ::reflpp::json::ToJson(stream3, v);

    std::cout << stream3 << std::endl;

    json_str = R"""(
{
    "123": 3.14,
    "456": false,
    "789": 111
}
    )""";

    ::reflpp::Value v1;
    ec = ::reflpp::json::FromJson(json_str, v1, &detail_emsg);
    std::cout << "err: " << detail_emsg << std::endl;

    auto& dict1 = *v1.ToDirectory();
    for (auto& kv : dict1) {
        std::cout << "key: " << kv.first << std::endl;
    }
    std::cout << "val: " << dict1["123"].As<::reflpp::Value::Float>() << std::endl;
    std::cout << "val: " << dict1["456"].As<::reflpp::Value::Boolean>() << std::endl;
    std::cout << "val: " << dict1["789"].As<::reflpp::Value::Int>() << std::endl;

    return 0;
}
