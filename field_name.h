#pragma once

#include <for_each.h>

#include <iostream>
#include <source_location>
#include <string_view>

namespace reflpp {
namespace _ {

template <typename T>
extern T FakeObject;

template <auto T>
constexpr const char* TypeToString() noexcept {
    return std::source_location::current().function_name();
}

constexpr std::string_view ExtractFieldName(
    std::string_view type_str) noexcept {
#if defined(__GNUC__)
    constexpr std::string_view kFakeObject = "FakeObject";
    constexpr std::string_view kDomainDesc = "::";

    auto start = type_str.find(kFakeObject);
    if (start == std::string_view::npos) {
        return {};
    }

    auto domain_start = type_str.find(kDomainDesc, start);
    if (start == std::string_view::npos) {
        return {};
    }
    domain_start += 2;  // skip domain descriptor

    auto end = domain_start;
    auto valid_identifier = [](char ch) -> bool {
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') ||
               (ch >= 'A' && ch <= 'Z') || ch == '_';
    };

    for (; end < type_str.size(); end++) {
        if (!valid_identifier(type_str[end])) {
            break;
        }
    }

    return type_str.substr(domain_start, end - domain_start);
#else
    // TODO: support clang compiler
    static_assert(false, "Unsupported compiler");
#endif
}

template <typename T, std::size_t... Is>
consteval auto GetFieldNamesImpl(std::index_sequence<Is...>) {
    constexpr std::size_t N = sizeof...(Is);
    if constexpr (N == 0) {
        return std::array<std::string_view, N>{};
    } else {
        constexpr auto tup = TieAsTuple<T, N>(FakeObject<T>);

        std::array<std::string_view, N> field_names;

        ((field_names[Is] =
              ExtractFieldName(TypeToString<std::get<Is>(tup)>())),
         ...);

        return field_names;
    }
}

}  // namespace _

template <typename T>
consteval auto GetFieldNames() {
    constexpr auto N = FieldsCount<T>();
    return _::GetFieldNamesImpl<T>(std::make_index_sequence<N>{});
}

template <typename T>
inline constexpr auto kFieldNames = GetFieldNames<T>();

template <typename T, std::size_t I>
consteval std::string_view GetFieldName() {
    static_assert(I < kFieldNames<T>.size(), "Index out of range");
    return kFieldNames<T>[I];
}

}  // namespace reflpp
