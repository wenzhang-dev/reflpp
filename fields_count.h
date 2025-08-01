#pragma once

#include <type_traits>
#include <utility>

namespace reflpp {
namespace _ {

struct AnyType {
    template <typename T>
    constexpr operator T() const {
        return T{};
    }
};

template <typename T, std::size_t... Is>
constexpr auto IsConstructibleImpl(std::index_sequence<Is...>)
    -> decltype(T{((void)Is, AnyType{})...}, std::true_type{}) {
    return {};
}

template <typename T, std::size_t... Is>
constexpr std::false_type IsConstructibleImpl(...) {
    return {};
}

template <typename T, std::size_t N>
constexpr bool IsConstructible =
    decltype(IsConstructibleImpl<T>(std::make_index_sequence<N>{}))::value;

template <typename T, std::size_t Min = 0, std::size_t Max = 513>
constexpr std::size_t AggregateFieldsCount() {
    static_assert(std::is_aggregate_v<T>,
                  "Only aggregated structs are supported");

    if constexpr (Min == Max) {
        return Min;
    }

    constexpr std::size_t mid = (Min + Max + 1) >> 1;
    if constexpr (IsConstructible<T, mid>) {
        return AggregateFieldsCount<T, mid, Max>();
    } else {
        return AggregateFieldsCount<T, Min, mid - 1>();
    }
}

}  // namespace _

// Some compile-times limit the number of expansions of a template and send a
// similar error message: constexpr evaluation depth exceeds maximum of 512 (use
// `-fconstexpr-depth=` to increase the maximum)
template <typename T, std::size_t Max = 512>
constexpr std::size_t FieldsCount() {
    constexpr auto count = _::AggregateFieldsCount<T, 0, Max + 1>();
    static_assert(count <= Max, "Too many fields");
    return count;
}

}  // namespace reflpp
