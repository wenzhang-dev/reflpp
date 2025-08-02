#pragma once

#include <system_error>

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

}  // namespace json
}  // namespace reflpp
