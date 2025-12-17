//
//  This file is part of ATLink.
//
//  ATLink is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ATLink is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with ATLink.  If not, see <https://www.gnu.org/licenses/>.
//

#pragma once

#include <charconv>
#include <gsl/span>
#include <string_view>

namespace ATL_NS {
namespace Core {

using ReadOnlyText = std::string_view;
using MutableBuffer = gsl::span<char>;

class AField {
  public:
    virtual size_t stringify(MutableBuffer output) const = 0;
    virtual size_t parse(ReadOnlyText input) = 0;
    virtual ~AField() = default;
};

class Sequence : public AField {
    const ReadOnlyText seq;

  public:
    explicit constexpr Sequence(ReadOnlyText seq) : seq{seq} {}

    size_t stringify(MutableBuffer output) const override {
        size_t n = 0U;
        if (seq.size() < output.size()) {
            std::copy_n(seq.data(), seq.size(), output.data());
            n = seq.size();
        }
        return n;
    }

    size_t parse(ReadOnlyText input) override {
        auto start = input.find(seq);
        size_t n = 0U;
        if (0U == start) {
            n = seq.size();
        }
        return n;
    }

    ReadOnlyText view() const {
        return seq;
    }

    size_t length() const {
        return seq.size();
    }
};

class TextBuilder {
  public:
    TextBuilder(MutableBuffer buf, size_t &len) : buf{buf.data()}, cap{buf.size()}, len{len} {}

    std::size_t size() const {
        return len;
    }
    std::size_t capacity() const {
        return cap;
    }

    void clear() {
        if (cap > 0)
            buf[0] = '\0';
        len = 0;
    }

    TextBuilder &operator<<(const char *s) {
        if (!s)
            return *this;
        while (*s && len < cap - 1) {
            buf[len++] = *s++;
        }
        buf[std::min(len, cap - 1)] = '\0';
        return *this;
    }

    TextBuilder &operator<<(char c) {
        if (len < cap - 1) {
            buf[len++] = c;
            buf[len] = '\0';
        }
        return *this;
    }

    TextBuilder &operator<<(bool v) {
        return (*this) << (v ? "true" : "false");
    }

    template <class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    TextBuilder &operator<<(T v) {
        if (len >= cap - 1)
            return *this;

        char *first = buf + len;
        char *last = buf + cap - 1; // reserve for '\0'
        auto rc = std::to_chars(first, last, v);
        if (rc.ec == std::errc{}) {
            len += static_cast<std::size_t>(rc.ptr - first);
            buf[len] = '\0';
        }
        return *this;
    }

  private:
    char *buf;
    std::size_t cap;
    std::size_t &len;
};

template <std::size_t N>
class LineText : public AField {
    std::array<char, N> chars{};
    size_t length{0U};
    TextBuilder textBuilder{chars, length};

  public:
    TextBuilder &builder() {
        return textBuilder;
    }

    size_t stringify(MutableBuffer output) const override {
        const std::size_t n = std::min<std::size_t>(length, output.size());
        std::copy_n(chars.data(), n, output.data());
        return n;
    }

    size_t parse(ReadOnlyText input) override {
        // TODO: fix it!!!
        auto pos = input.find("\r\n");
        std::size_t take = 0U;
        if (pos == std::string_view::npos) {
            take = std::min<std::size_t>(input.size(), chars.size());
        } else {
            take = std::min<std::size_t>(pos, chars.size());
        }

        std::copy_n(input.data(), take, chars.data());
        chars[take] = '\0';
        length = take;
        return length;
    }

    ReadOnlyText view() const {
        return ReadOnlyText{chars.data(), length};
    }
};

template <std::size_t N>
class QuotedText : public AField {
    std::array<char, N> chars{};
    size_t length{0U};
    TextBuilder textBuilder{chars, length};

  public:
    TextBuilder &builder() {
        return textBuilder;
    }

    size_t stringify(MutableBuffer output) const override {
        std::size_t extra = 0U;
        for (char c : chars)
            extra += (('\"' == c) ? 1 : 0);

        size_t n = 0;
        const std::size_t need = 2U + length + extra;
        if (need < output.size()) {
            output[n++] = '\"';
            for (auto i = 0U; i < length; ++i) {
                auto c = chars[i];
                if (c == '\"') {
                    output[n++] = '\\';
                }
                output[n++] = c;
            }
            output[n++] = '\"';
            output[n] = '\0';
        }
        return n;
    }

    size_t parse(ReadOnlyText input) override {
        length = 0U;

        if (input.empty() || input[0] != '\"') {
            return 0U;
        }

        std::size_t in = 1U;  // index in input, skip opening quote
        std::size_t out = 0U; // index in chars

        while (in < input.size()) {
            const char c = input[in];

            // Closing quote → done
            if (c == '\"') {
                if (!chars.empty()) {
                    chars[std::min(out, chars.size() - 1)] = '\0';
                }
                length = out;
                return in + 1U; // total consumed, incl. closing quote
            }

            // Handle escaped quote \" → "
            if (c == '\\') {
                if (in + 1U < input.size() && input[in + 1U] == '\"') {
                    if (out + 1U >= chars.size()) {
                        return 0U; // no space (reserve for '\0')
                    }
                    chars[out++] = '\"';
                    in += 2U;
                    continue;
                }
                // Any other backslash: copy as-is
                if (out + 1U >= chars.size()) {
                    return 0U;
                }
                chars[out++] = '\\';
                ++in;
                continue;
            }

            // Normal character
            if (out + 1U >= chars.size()) {
                return 0U;
            }
            chars[out++] = c;
            ++in;
        }

        // Unterminated quoted string
        return 0U;
    }

    ReadOnlyText view() const {
        return ReadOnlyText{chars.data(), length};
    }
};

} // namespace Core
} // namespace ATL_NS
