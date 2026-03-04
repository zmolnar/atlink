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

namespace ATL_NS {
namespace Utils {

class Deserializer : public Core::AResponseVisitor {

    Core::ReadOnlyText input;
    size_t length = 0;

  public:
    explicit Deserializer(Core::ReadOnlyText input) : input(input) {}
    ~Deserializer() = default;

    void rewind() override {
        length = 0U;
    }

    bool visit(Core::AField &field) override {
        skipWhitespaces();
        auto n = field.parse(input.substr(length));
        length += n;
        return (0U < n);
    }

    bool visit(int &i) override {
        skipWhitespaces();
        const auto *first = input.data() + length;
        const auto *last = input.data() + input.size();
        int num = 0U;
        auto result = std::from_chars(first, last, num);
        auto success = (result.ec == std::errc{});
        if (success) {
            i = num;
            length += result.ptr - (input.data() + length);
        }
        return success;
    }

    size_t consumed() const override {
        return length;
    }

  private:
    void skipWhitespaces() {
        auto trimmed_input = input.substr(length);
        auto start = trimmed_input.find_first_not_of(" \t");

        if (start != std::string_view::npos) {
            length += start;
        } else {
            length = input.size();
        }
    }
};

} // namespace Utils
} // namespace ATL_NS