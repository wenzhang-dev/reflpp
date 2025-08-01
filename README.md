# reflpp

reflpp is a lightweight, non-intrusive reflection library. It is implemented based on cxx20. The library makes it easy to iterate over the members of a struct, get the number of members, get the name of the member, and json serialization and deserialization. The most important feature is that it is non-intrusive. There is no need to use macros or registers to declare which fields the struct has, the address offset of each field.

While everything looks nice, it's limited to the aggregate type.

# Dependency

- cxx20
- fmtlib

# Example

```c++
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <reflpp.h>

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

struct Bar {
    Foo foo;

    double f64;
    std::string str;
};

int main() {
    std::cout << "The number of fields for Foo: "
              << ::reflpp::FieldsCount<Foo>() << std::endl;
    std::cout << "The number of fields for Bar: "
              << ::reflpp::FieldsCount<Bar>() << std::endl;

    std::cout << "The field names of Foo: "
              << fmt::format("{}", ::reflpp::kFieldNames<Foo>) << std::endl;
    std::cout << "The field names of Bar: "
              << fmt::format("{}", ::reflpp::kFieldNames<Bar>) << std::endl;

    // clang-format off
    Bar bar{
        .foo={
            1,
            false,
            3.14,
            "456",
            {"789", "123", "456"},
            {7, 8, 9},
            {std::make_pair("1", "2"), std::make_pair("3", "4")},
            {7, 8, 9},
            1,
            std::make_unique<std::string>("234")
        },
        .f64=1.23,
        .str="789"
    };
    // clang-format on

    std::cout << "Iterate Foo struct: " << std::endl;
    ::reflpp::ForEach(bar.foo, [](auto idx, const auto& v) {
        std::cout << fmt::format("The field idx: {}, value: {}", idx, v)
                  << std::endl;
    });

    std::cout << "Json serialization: " << std::endl;

    ::reflpp::json::CompactJsonFormatter stream;
    ::reflpp::json::ToJson(stream, bar);

    std::cout << stream << std::endl;

    std::cout << "Json deserialization: " << std::endl;
    Bar bar1;

    std::string_view json_str = R"""(
    {"foo":{"a":1,"b":false,"c":3.14,"d":"456","e":["789","123","456"],"f":[7,8,9],"g":{"1":"2","3":"4"},"h":[7,8,9],"i":1,"j":"234"},"f64":1.23,"str":"789"}
    )""";
    auto ec = ::reflpp::json::FromJson(json_str, bar1);
    REFLPP_ASSERT(!ec);

    ::reflpp::ForEach(bar1.foo, [](auto idx, const auto& v) {
        std::cout << fmt::format("The field idx: {}, value: {}", idx, v)
                  << std::endl;
    });

    return 0;
}
```

The output from the example program is as follows:

```txt
The number of fields for Foo: 10
The number of fields for Bar: 3
The field names of Foo: ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j"]
The field names of Bar: ["foo", "f64", "str"]
Iterate Foo struct:
The field idx: 0, value: 1
The field idx: 1, value: false
The field idx: 2, value: 3.14
The field idx: 3, value: 456
The field idx: 4, value: ["789", "123", "456"]
The field idx: 5, value: [7, 8, 9]
The field idx: 6, value: {"1": "2", "3": "4"}
The field idx: 7, value: {7, 8, 9}
The field idx: 8, value: optional(1)
The field idx: 9, value: 234
Json serialization:
{"foo":{"a":1,"b":false,"c":3.14,"d":"456","e":["789","123","456"],"f":[7,8,9],"g":{"1":"2","3":"4"},"h":[7,8,9],"i":1,"j":"234"},"f64":1.23,"str":"789"}
Json deserialization:
The field idx: 0, value: 1
The field idx: 1, value: false
The field idx: 2, value: 3.14
The field idx: 3, value: 456
The field idx: 4, value: ["789", "123", "456"]
The field idx: 5, value: [7, 8, 9]
The field idx: 6, value: {"1": "2", "3": "4"}
The field idx: 7, value: {7, 8, 9}
The field idx: 8, value: optional(1)
The field idx: 9, value: 234
```
