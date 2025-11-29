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

#include "atlink/core/Packet.h"

#include <charconv>
#include <cstring>

namespace ATL_NS {
namespace Utils {

class Serializer : public Core::ACommandVisitor {
    const Core::MutableBuffer buf;
    Core::MutableBuffer rest;

  public:
    explicit Serializer(Core::MutableBuffer output) : buf(output), rest{output} {
        memset(rest.data(), 0U, rest.size());
    }

    bool visit(const Core::Sequence &s) override {
        auto n = s.stringify(rest);
        rest = rest.subspan(n);
        return (0U < n);
    }

    bool visit(const Core::QuotedStringView s) override {
        auto n = writeQuoted(rest, s);
        rest = rest.subspan(n);
        return (0U < n);
    }

    bool visit(const Core::AEnum &e) override {
        auto n = e.stringify(rest);
        rest = rest.subspan(n);
        return (0U < n);
    }

    bool visit(int i) override {
        char *first = rest.data();
        char *last = rest.data() + rest.size();
        auto rc = std::to_chars(first, last, i);
        auto success = (std::errc{} == rc.ec);
        if (success) {
            auto len = static_cast<std::size_t>(rc.ptr - first);
            rest = rest.subspan(len);
        }
        return success;
    }

    std::size_t written() const override {
        return buf.size() - rest.size();
    }

    Core::ReadOnlyText output() const {
        return Core::ReadOnlyText{buf.data(), written()};
    }

  private:
    size_t writeQuoted(Core::MutableBuffer out, Core::ReadOnlyText txt) {
        std::size_t extra = 0U;
        for (char c : txt)
            extra += (('\"' == c) ? 1 : 0);

        size_t n = 0;
        const std::size_t need = 2U + txt.size() + extra;
        if (need < out.size()) {
            out[n++] = '\"';
            for (char c : txt) {
                if (c == '\"') {
                    out[n++] = '\\';
                }
                out[n++] = c;
            }
            out[n++] = '\"';
        }

        return n;
    }
};

} // namespace Utils
} // namespace ATL_NS