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

class Serializer : public Core::AOutputVisitor {
    const Core::MutableBuffer buf;
    Core::MutableBuffer rest;

    bool valid = true;

  public:
    explicit Serializer(Core::MutableBuffer output) : buf(output), rest{output} {
        memset(rest.data(), 0U, rest.size());
    }

    void reset() override {
        valid = true;
        rest = buf;
    }

    bool visit(const Core::Tag &tag) override {
        if (valid) {
            auto n = tag.stringify(rest);
            valid = (0U < n);
            rest = rest.subspan(n);
        }
        return valid;
    }

    bool visit(const Core::Comma &comma) override {
        if (valid) {
            auto n = comma.stringify(rest);
            valid = (0U < n);
            rest = rest.subspan(n);
        }
        return valid;
    }

    bool visit(const Core::Term &term) override {
        if (valid) {
            auto n = term.stringify(rest);
            valid = (0U < n);
            rest = rest.subspan(n);
        }
        return valid;
    }

    bool visit(const Core::ReadOnlyText s) override {
        if (valid) {
            auto n = writeQuoted(rest, s);
            valid = (0U < n);
            rest = rest.subspan(n);
        }
        return valid;
    }

    bool visit(const Core::AEnum &e) override {
        if (valid) {
            auto n = e.stringify(rest);
            valid = (0U < n);
            rest = rest.subspan(n);
        }
        return valid;
    }

    bool visit(int i) override {
        if (valid) {
            char *first = rest.data();
            char *last = rest.data() + rest.size();
            auto rc = std::to_chars(first, last, i);
            valid = (std::errc{} == rc.ec);
            if (std::errc{} == rc.ec) {
                valid = true;
                auto len = static_cast<std::size_t>(rc.ptr - first);
                rest = rest.subspan(len);
            }
        }
        return valid;
    }

    bool isValid() const {
        return valid;
    }

    std::size_t numberOfBytesWritten() const {
        return buf.size() - rest.size();
    }

    Core::ReadOnlyText output() const {
        return Core::ReadOnlyText {buf.data(), numberOfBytesWritten()};
    }

  private:
    size_t writeQuoted(Core::MutableBuffer out, Core::ReadOnlyText str) {

        std::size_t extra = 0U;
        for (char c : str)
            extra += (('\"' == c) ? 1 : 0);

        size_t n = 0;
        const std::size_t need = 2U + str.size() + extra;
        if (need < out.size()) {
            out[n++] = '\"';
            for (char c : str) {
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