#pragma once

#include <cstring>
#include <string>

namespace reflpp {
namespace json {
namespace _ {
template <typename Stream, bool Pretty = false, std::size_t Indent = 4>
class Formatter : public Stream {
   public:
    using value_type = typename Stream::value_type;
    using size_type = typename Stream::size_type;

    using Stream::reserve;

    Formatter() = default;
    Formatter(size_type init_cap) { Stream::Reserve(init_cap); }

    Formatter(Formatter&&) = default;
    Formatter(const Formatter&) = delete;
    Formatter& operator=(Formatter&&) = default;
    Formatter& operator=(const Formatter&) = delete;

   public:
    Formatter& append(size_type count, value_type ch) {
        if constexpr (!Pretty) {
            Stream::append(count, ch);
        } else {
            for (size_type i = 0; i < count; ++i) {
                PrettyFormat(ch);
            }
        }

        return *this;
    }

    Formatter& append(const value_type* s) {
        if constexpr (!Pretty) {
            Stream::append(s);
        } else {
            for (std::size_t i = 0; i < std::strlen(s); ++i) {
                PrettyFormat(s[i]);
            }
        }

        return *this;
    }

    Formatter& append(const value_type* s, size_type count) {
        if constexpr (!Pretty) {
            Stream::append(s, count);
        } else {
            for (size_type i = 0; i < count; ++i) {
                PrettyFormat(s[i]);
            }
        }

        return *this;
    }

    void push_back(value_type c) {
        if constexpr (!Pretty) {
            Stream::push_back(c);
        } else {
            PrettyFormat(c);
        }
    }

    Stream& stream() { return *this; }
    const Stream& stream() const { return *this; }

   private:
    enum State {
        kNormal,
        kEscaped,
        kString,
        kBeforeAsterisk,
        kComment,
        kBeforeFlash,
    };

    void NewLine() {
        if constexpr (Pretty) {
            Stream::push_back('\n');
            Stream::append(indent_ * Indent, ' ');
        }
    }

    void CommentStart() {
        if constexpr (Pretty) {
            Stream::append(" /");
        } else {
            Stream::push_back("/");
        }
    }

    void ValueStart() {
        if constexpr (Pretty) {
            Stream::append(": ");
        } else {
            Stream::push_back(":");
        }
    }

    void FormatChar(char c) {
        switch (c) {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
                break;
            case ',':
                Stream::push_back(c);
                NewLine();
                break;
            case '[':
            case '{':
                Stream::push_back(c);
                ++indent_;
                NewLine();
                break;
            case ']':
            case '}':
                --indent_;
                NewLine();
                Stream::push_back(c);
                break;
            case '\\':
                Stream::push_back(c);
                state_ = State::kEscaped;
                break;
            case '\"':
                Stream::push_back(c);
                state_ = State::kString;
                break;
            case '/':
                CommentStart();
                state_ = State::kBeforeAsterisk;
                break;
            case ':':
                ValueStart();
                break;
            default:
                Stream::push_back(c);
                break;
        }
    }

    void FormatOther(char c) {
        switch (state_) {
            case State::kEscaped:
                state_ = State::kNormal;
                break;
            case State::kString:
                if (c == '\"') {
                    state_ = State::kNormal;
                }
                break;
            case State::kBeforeAsterisk:
                state_ = State::kComment;
                break;
            case State::kComment:
                if (c == '*') {
                    state_ = State::kBeforeFlash;
                }
                break;
            case State::kBeforeFlash:
                state_ = (c == '/') ? State::kNormal : State::kComment;
                break;
            default:
                break;
        }
    }

    void PrettyFormat(char c) {
        if (state_ == State::kNormal) {
            FormatChar(c);
        } else {
            Stream::push_back(c);
            FormatOther(c);
        }
    }

    State state_{kNormal};
    std::size_t indent_{0};
};
}  // namespace _

using PrettyJsonFormatter = _::Formatter<std::string, true, 4>;
using CompactJsonFormatter = _::Formatter<std::string, false, 4>;

}  // namespace json
}  // namespace reflpp
