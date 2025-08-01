#pragma once

#include <fields_count.h>
#include <type_trait.h>

#include <functional>
#include <tuple>

#define REFLPP_FOR_EACH_CNT(MACRO) \
    MACRO(1)                       \
    MACRO(2)                       \
    MACRO(3)                       \
    MACRO(4)                       \
    MACRO(5)                       \
    MACRO(6)                       \
    MACRO(7)                       \
    MACRO(8)                       \
    MACRO(9)                       \
    MACRO(10)                      \
    MACRO(11)                      \
    MACRO(12)                      \
    MACRO(13)                      \
    MACRO(14)                      \
    MACRO(15)                      \
    MACRO(16)                      \
    MACRO(17)                      \
    MACRO(18)                      \
    MACRO(19)                      \
    MACRO(20)                      \
    MACRO(21)                      \
    MACRO(22)                      \
    MACRO(23)                      \
    MACRO(24)                      \
    MACRO(25)                      \
    MACRO(26)                      \
    MACRO(27)                      \
    MACRO(28)                      \
    MACRO(29)                      \
    MACRO(30)                      \
    MACRO(31)                      \
    MACRO(32)

#define REFLPP_DECL_TIE_AS_TUPLE(N)                                          \
    template <typename T, typename... Args>                                  \
    constexpr auto TieAsTupleImpl(T& obj,                                    \
                                  std::integral_constant<std::size_t, N>) {  \
        auto& [REFLPP_UNPACK_FIELDS_##N] =                                   \
            const_cast<std::remove_cv_t<T>&>(obj);                           \
        return [](auto&... args) {                                           \
            return std::make_tuple(WorkaroundWrap(std::addressof(args))...); \
        }(REFLPP_UNPACK_FIELDS_##N);                                         \
    }

// clang-format off
#define REFLPP_UNPACK_FIELDS_1   a1
#define REFLPP_UNPACK_FIELDS_2   a1, a2
#define REFLPP_UNPACK_FIELDS_3   a1, a2, a3
#define REFLPP_UNPACK_FIELDS_4   a1, a2, a3, a4
#define REFLPP_UNPACK_FIELDS_5   a1, a2, a3, a4, a5
#define REFLPP_UNPACK_FIELDS_6   a1, a2, a3, a4, a5, a6
#define REFLPP_UNPACK_FIELDS_7   a1, a2, a3, a4, a5, a6, a7
#define REFLPP_UNPACK_FIELDS_8   a1, a2, a3, a4, a5, a6, a7, a8
#define REFLPP_UNPACK_FIELDS_9   a1, a2, a3, a4, a5, a6, a7, a8, a9
#define REFLPP_UNPACK_FIELDS_10  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10
#define REFLPP_UNPACK_FIELDS_11  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11
#define REFLPP_UNPACK_FIELDS_12  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12
#define REFLPP_UNPACK_FIELDS_13  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13
#define REFLPP_UNPACK_FIELDS_14  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14
#define REFLPP_UNPACK_FIELDS_15  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15
#define REFLPP_UNPACK_FIELDS_16  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16
#define REFLPP_UNPACK_FIELDS_17  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17
#define REFLPP_UNPACK_FIELDS_18  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18
#define REFLPP_UNPACK_FIELDS_19  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19
#define REFLPP_UNPACK_FIELDS_20  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20
#define REFLPP_UNPACK_FIELDS_21  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21
#define REFLPP_UNPACK_FIELDS_22  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22
#define REFLPP_UNPACK_FIELDS_23  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23
#define REFLPP_UNPACK_FIELDS_24  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24
#define REFLPP_UNPACK_FIELDS_25  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25
#define REFLPP_UNPACK_FIELDS_26  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26
#define REFLPP_UNPACK_FIELDS_27  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27
#define REFLPP_UNPACK_FIELDS_28  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28
#define REFLPP_UNPACK_FIELDS_29  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29
#define REFLPP_UNPACK_FIELDS_30  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30
#define REFLPP_UNPACK_FIELDS_31  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31
#define REFLPP_UNPACK_FIELDS_32  a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32
// clang-format on

namespace reflpp {
namespace _ {

template <typename T>
struct Wrapper {
    T value;
};

template <typename T>
constexpr auto WorkaroundWrap(const T& arg) noexcept {
    return Wrapper<T>{arg};
}

REFLPP_FOR_EACH_CNT(REFLPP_DECL_TIE_AS_TUPLE);

// speficication to support empty struct
template <typename T>
constexpr auto TieAsTupleImpl(T&, std::integral_constant<std::size_t, 0>) {
    return std::tuple<>();
}

template <typename T, std::size_t N = FieldsCount<T>()>
constexpr auto TieAsTuple(T& obj) {
    return TieAsTupleImpl(obj, std::integral_constant<std::size_t, N>{});
}

template <typename T, typename F, std::size_t... Is>
constexpr void ForEachImpl(T& obj, F&& func, std::index_sequence<Is...>) {
    // auto t = TieAsTuple(obj);
    (func(Is, *(std::get<Is>(TieAsTuple(obj))).value), ...);
}

template <typename T, typename F, std::size_t... Is>
constexpr void ForEachTupleImpl(T& obj, F&& func, std::index_sequence<Is...>) {
    (func(Is, std::get<Is>(obj)), ...);
}

}  // namespace _

template <typename T, typename F, std::enable_if_t<!IsTuple<T>, int> _ = 0>
constexpr void ForEach(const T& obj, F&& f) {
    constexpr std::size_t N = FieldsCount<T>();
    _::ForEachImpl(obj, std::forward<F>(f), std::make_index_sequence<N>{});
}

template <typename T, typename F, std::enable_if_t<IsTuple<T>, int> _ = 0>
constexpr void ForEach(const T& obj, F&& f) {
    constexpr std::size_t N = std::tuple_size_v<T>;
    return _::ForEachTupleImpl(obj, std::forward<F>(f),
                               std::make_index_sequence<N>{});
}

}  // namespace reflpp
