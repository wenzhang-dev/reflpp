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

class ComplexBaseClass {
   public:
    virtual ~ComplexBaseClass() {}
    virtual void DoSomething() = 0;
};

class ComplexClass : public ComplexBaseClass {
   public:
    ComplexClass(int a, bool b, double c, std::string_view d)
        : a_(a), b_(b), c_(c), d_(d) {}

    void DoSomething() override {
        std::cout << a_ << b_ << c_ << d_ << std::endl;
    }

    void Dump(::reflpp::Value& v) {
        auto& d = v.AsDirectory();
        d["a"] = a_;
        d["b"] = b_;
        d["c"] = c_;
        d["d"] = d_;
    }

   private:
    int a_;
    bool b_;
    double c_;
    std::string d_;
};

int main() {
    std::cout << "Json serialization: " << std::endl;

    ComplexClass cc(1, false, 3.14, "456");
    ::reflpp::Value v;

    cc.Dump(v);

    ::reflpp::json::CompactJsonFormatter stream;
    ::reflpp::json::ToJson(stream, v);

    std::cout << stream << std::endl;

    std::cout << "Json deserialization" << std::endl;

    ::reflpp::Value v1;
    auto json_str = R"""({"a":1,"b":false,"c":3.14,"d":"456"})""";
    auto ec = ::reflpp::json::FromJson(json_str, v1);
    REFLPP_ASSERT(!ec);

    auto& dict = *v1.ToDirectory();
    for(auto& kv : dict) {
        std::cout << "key: " << kv.first << std::endl;
    }

    return 0;
}
