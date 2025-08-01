#pragma once

#include <charconv>
#include <optional>
#include <string_view>
#include <system_error>

#define REFLPP_ASSERT(expr)                                              \
    do {                                                                 \
        if (!(expr)) {                                                   \
            std::cerr << "Assertion failed: " << #expr << ", file "      \
                      << __FILE__ << ", line " << __LINE__ << std::endl; \
            std::abort();                                                \
        }                                                                \
    } while (0)

namespace reflpp {

template <typename T>
std::error_code LexicalCast(std::string_view str, T& val) {
    auto res = std::from_chars(str.data(), str.data() + str.size(), val);
    return std::make_error_code(res.ec);
}

}  // namespace reflpp
