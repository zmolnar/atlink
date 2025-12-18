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

    bool visit(const Core::AElement &field) override {
        auto n = field.stringify(rest);
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
};

} // namespace Utils
} // namespace ATL_NS