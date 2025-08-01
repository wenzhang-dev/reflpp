#pragma once

#include <field_name.h>
#include <fmt/format.h>
#include <for_each.h>
#include <type_trait.h>

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace reflpp {
namespace json {

namespace _ {

template <typename Stream, typename It, typename T, typename F>
inline void Join(Stream& ss, It first, It last, const T& delim, const F& f) {
    if (first == last) return;

    f(*first++);
    while (first != last) {
        ss.push_back(delim);
        f(*first++);
    }
}

}  // namespace _

template <typename Stream, typename T>
inline void FormatJsonValue(Stream& ss, const std::optional<T>&);

template <typename Stream, typename T,
          std::enable_if_t<IsMapContainer<T>, int> = 0>
inline void FormatJsonValue(Stream& ss, const T&);

template <typename Stream, typename T,
          std::enable_if_t<IsSequenceContainer<T>, int> = 0>
inline void FormatJsonValue(Stream& ss, const T&);

template <typename Stream, typename T, std::enable_if_t<IsTuple<T>, int> = 0>
inline void FormatJsonValue(Stream& s, const T&);

template <typename Stream, typename T, std::enable_if_t<IsVariant<T>, int> = 0>
inline void FormatJsonValue(Stream& s, const T&);

template <typename Stream, typename T, std::enable_if_t<IsIntegral<T>, int> = 0>
inline void FormatJsonValue(Stream& ss, T value);

template <typename Stream, typename T,
          std::enable_if_t<IsNonCharArray<T>, int> = 0>
inline void FormatJsonValue(Stream& ss, const T& v);

template <typename Stream, typename T,
          std::enable_if_t<IsCharArray<T>, int> = 0>
inline void FormatJsonValue(Stream& ss, const T& v);

template <typename Stream>
inline void FormatJsonValue(Stream& s, std::nullptr_t) {
    s.append("null");
}

template <typename Stream>
inline void FormatJsonValue(Stream& s, bool b) {
    s.append(b ? "true" : "false");
};

template <typename Stream>
inline void FormatJsonValue(Stream& s, char value) {
    s.append("\"");
    s.push_back(value);
    s.append("\"");
}

template <typename Stream, typename T, std::enable_if_t<IsIntegral<T>, int>>
inline void FormatJsonValue(Stream& s, T value) {
    fmt::format_to(std::back_inserter(s), "{}", value);
}

template <typename Stream, typename T, std::enable_if_t<IsFloat<T>, int> = 0>
inline void FormatJsonValue(Stream&& s, T value) {
    fmt::format_to(std::back_inserter(s), "{}", value);
}

template <typename Stream, typename T,
          std::enable_if_t<IsStringLike<T>, int> = 0>
inline void FormatJsonValue(Stream& s, T&& t) {
    s.push_back('"');
    s.append(t.data(), t.size());
    s.push_back('"');
}

template <typename Stream, typename T>
inline void FormatJsonValue(Stream& s, const std::optional<T>& val) {
    if (!val) {
        s.append("null");
    } else {
        FormatJsonValue(s, *val);
    }
}

namespace _ {

// notes, for json object, the key may be numeric. in this case, we should
// format it as a string
template <typename Stream, typename T, std::enable_if_t<IsNumeric<T>, int> = 0>
inline void FormatJsonKey(Stream& s, T& t) {
    s.push_back('"');
    FormatJsonValue(s, t);
    s.push_back('"');
}

template <typename Stream, typename T,
          std::enable_if_t<IsStringLike<T>, int> = 0>
inline void FormatJsonKey(Stream& s, T&& t) {
    FormatJsonValue(s, std::forward<T>(t));
}

}  // namespace _

template <typename Stream, typename T, std::enable_if_t<IsNonCharArray<T>, int>>
inline void FormatJsonValue(Stream& ss, const T& v) {
    ss.push_back('[');
    _::Join(ss, std::begin(v), std::end(v), ',',
            [&ss](const auto& jsv) constexpr { FormatJsonValue(ss, jsv); });
    ss.push_back(']');
}

// notes, in c-style programming, we usually use a char array to store a string
// so, we should handle char[] as a string
template <typename Stream, typename T, std::enable_if_t<IsCharArray<T>, int>>
inline void FormatJsonValue(Stream& ss, const T& v) {
    constexpr size_t n = sizeof(T) / sizeof(decltype(std::declval<T>()[0]));
    ss.push_back('"');
    auto get_length = [&v](int n) constexpr {
        for (int i = 0; i < n; ++i) {
            if (v[i] == '\0') return i;
        }
        return n;
    };
    size_t len = get_length(n);
    ss.append(std::begin(v), len);
    ss.push_back('"');
}

template <typename Stream, typename T, std::enable_if_t<IsMapContainer<T>, int>>
inline void FormatJsonValue(Stream& s, const T& v) {
    s.push_back('{');
    _::Join(s, v.cbegin(), v.cend(), ',', [&s](const auto& jsv) constexpr {
        _::FormatJsonKey(s, jsv.first);
        s.push_back(':');
        FormatJsonValue(s, jsv.second);
    });
    s.push_back('}');
}

template <typename Stream, typename T,
          std::enable_if_t<IsSetContainer<T>, int> = 0>
inline void FormatJsonValue(Stream& s, const T& v) {
    s.push_back('[');
    _::Join(s, v.cbegin(), v.cend(), ',',
            [&s](const auto& jsv) constexpr { FormatJsonValue(s, jsv); });
    s.push_back(']');
}

template <typename Stream, typename T,
          std::enable_if_t<IsSequenceContainer<T>, int>>
inline void FormatJsonValue(Stream& s, const T& v) {
    s.push_back('[');
    _::Join(s, v.cbegin(), v.cend(), ',',
            [&s](const auto& jsv) constexpr { FormatJsonValue(s, jsv); });
    s.push_back(']');
}

template <typename Stream, typename T, std::enable_if_t<IsSmartPtr<T>, int> = 0>
inline void FormatJsonValue(Stream& s, const T& v) {
    if (v) {
        FormatJsonValue(s, *v);
    } else {
        s.append("null");
    }
}

template <typename Stream, typename T, std::enable_if_t<IsTuple<T>, int>>
inline void FormatJsonValue(Stream& s, const T& t) {
    using U = typename std::decay_t<T>;
    constexpr std::size_t size = std::tuple_size_v<U>;
    s.push_back('[');

    ForEach(t, [&s, size](auto idx, const auto& v) constexpr {
        FormatJsonValue(s, v);
        if (idx != size - 1) {
            s.push_back(',');
        }
    });

    s.push_back(']');
}

template <typename Stream, typename T, std::enable_if_t<IsVariant<T>, int>>
inline void FormatJsonValue(Stream& s, const T& t) {
    std::visit([&s](auto value) { FormatJsonValue(s, value); }, t);
}

template <typename Stream, typename T,
          std::enable_if_t<IsAggregateStruct<T>, int> _ = 0>
inline void FormatJsonValue(Stream& s, const T& t) {
    s.push_back('{');

    constexpr auto& fields = ::reflpp::kFieldNames<T>;
    ForEach(t, [&](auto idx, const auto& v) constexpr {
        _::FormatJsonKey(s, fields[idx]);
        s.push_back(':');
        FormatJsonValue(s, v);
        if (idx < fields.size() - 1) {
            s.push_back(',');
        }
    });

    s.push_back('}');
}

template <typename Stream, typename T,
          std::enable_if_t<IsAggregateStruct<T>, int> _ = 0>
inline void ToJson(Stream& s, const T& t) {
    FormatJsonValue(s, t);
}

}  // namespace json
}  // namespace reflpp
