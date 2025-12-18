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

#include "atlink/core/BasicTypes.h"

namespace ATL_NS {
namespace Core {

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

} // namespace Core
} // namespace ATL_NS
