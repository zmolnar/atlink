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
#include <string_view>

namespace ATL_NS {
namespace Utils {

class Deserializer : public Core::AInputVisitor {

    std::string_view input;
    size_t length = 0;
    bool valid = true;

  public:
    explicit Deserializer(std::string_view input) : input(input) {}
    ~Deserializer() = default;

    void visit(const Core::Tag &tag) override {
        length = 0U;
        valid = true;
        skipWhitespaces();
        parse(tag);
    }

    void visit(const Core::Comma &comma) override {
        skipWhitespaces();
        parse(comma);
    }

    void visit(const Core::Term &term) override {
        skipWhitespaces();
        parse(term);
    }

    void visit(Core::MutableBuffer &str) override {
        skipWhitespaces();
        return;
    }

    void visit(Core::AEnum &e) override {
        skipWhitespaces();
        auto n = e.parse(input.substr(length));
        if (valid && (0U < n)) {
            length += n;
        } else {
            valid = false;
        }
    }

    void visit(int &i) override {
        skipWhitespaces();
        const auto *first = input.data() + length;
        const auto *last = input.data() + input.size();
        int num = 0U;
        auto result = std::from_chars(first, last, num);

        if (result.ec == std::errc{}) {
            i = num;
            length += result.ptr - (input.data() + length);
        } else {
            valid = false;
        }
        return;
    }

    bool isValid() const {
        return valid;
    }

    size_t numberOfBytesConsumed() const {
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
    void parse(const Core::Sequence &seq) {
        auto n = seq.parse(input.substr(length));
        if (valid && (0U < n)) {
            length += n;
        } else {
            valid = false;
            length = 0U;
        }
    }
};

} // namespace Utils
} // namespace ATL_NS