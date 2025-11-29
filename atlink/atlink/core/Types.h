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

struct LineText {
    MutableBuffer buf;
};

class Sequence {
    const ReadOnlyText seq;

  public:
    explicit constexpr Sequence(ReadOnlyText seq) : seq{seq} {}

    size_t stringify(MutableBuffer output) const {
        size_t n = 0U;
        if (seq.size() < output.size()) {
            std::copy_n(seq.data(), seq.size(), output.data());
            n = seq.size();
        }
        return n;
    }

    size_t parse(ReadOnlyText input) const {
        auto start = input.find(seq);
        size_t n = 0U;
        if (0U == start) {
            n = seq.size();
        }
        return n;
    }

    size_t length() const {
        return seq.size();
    }
};

class FixedBufStream {
  public:
    FixedBufStream(char *data, std::size_t size) : buf{data}, cap{size}, len{0} {}

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

    FixedBufStream &operator<<(const char *s) {
        if (!s)
            return *this;
        while (*s && len < cap - 1) {
            buf[len++] = *s++;
        }
        buf[std::min(len, cap - 1)] = '\0';
        return *this;
    }

    FixedBufStream &operator<<(char c) {
        if (len < cap - 1) {
            buf[len++] = c;
            buf[len] = '\0';
        }
        return *this;
    }

    FixedBufStream &operator<<(bool v) {
        return (*this) << (v ? "true" : "false");
    }

    template <class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    FixedBufStream &operator<<(T v) {
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
    std::size_t len;
};

using QuotedStringView = ReadOnlyText;
using QuotedStringStorage = MutableBuffer;

template <std::size_t N>
class QuotedField {
  public:
    QuotedField() : fb{chars.data(), chars.size()} {
        clear();
    }

    QuotedStringStorage storage() {
        return chars;
    }

    QuotedStringView view() const {
        return data();
    }

    FixedBufStream &stream() {
        return fb;
    }

    void clear() {
        fb.clear();
    }

    std::string_view data() const {
        auto len = strnlen(chars.data(), chars.size());
        return std::string_view{chars.data(), len};
    }

  private:
    std::array<char, N> chars{};
    FixedBufStream fb;
};

} // namespace Core
} // namespace ATL_NS
