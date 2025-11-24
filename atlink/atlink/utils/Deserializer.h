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

class Deserializer : public Core::AInputVisitor {

    Core::ReadOnlyText input;
    size_t length = 0;
    bool valid = true;

  public:
    explicit Deserializer(Core::ReadOnlyText input) : input(input) {}
    ~Deserializer() = default;

    void reset() override {
        length = 0U;
        valid = true;
    }

    bool visit(const Core::Sequence &seq) override {
        skipWhitespaces();
        if (valid && (0U < seq.length())) {
            auto n = seq.parse(input.substr(length));
            valid = (0U < n) || seq.isOptional();
            length += n;
        }
        return valid;
    }

    bool visit(Core::QuotedString &str) override {
        skipWhitespaces();
        std::string_view in = input.substr(length);

        auto consumed = parseStringLiteral(in, str.buf);
        valid = valid && (0U < consumed);
        length += consumed;
        return valid;
    }

    bool visit(Core::RawUntilTerm &raw) override {
        std::array<char, 3U> term{};
        auto success = Core::Term{}.stringify(term);
        assert(success);

        std::string_view in = input.substr(length);
        auto pos = in.find(term.data());

        std::size_t take = 0U;
        if (pos == std::string_view::npos) {
            take = std::min<std::size_t>(in.size(), raw.buf.size());
        } else {
            take = std::min<std::size_t>(pos, raw.buf.size());
        }

        for (std::size_t i = 0; i < take; ++i) {
            raw.buf[i] = in[i];
        }

        length += take;
        return valid;
    }

    bool visit(Core::AEnum &e) override {
        skipWhitespaces();
        auto n = e.parse(input.substr(length));
        if (valid && (0U < n)) {
            length += n;
        } else {
            valid = false;
        }
        return valid;
    }

    bool visit(int &i) override {
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
        return valid;
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

    static size_t parseStringLiteral(std::string_view in, gsl::span<char> out) {
        static constexpr auto npos = std::string_view::npos;
        auto start = in.find("\"");
        auto end = (npos != start) ? in.find("\"", start + 1U) : npos;
        size_t length = 0U;
        if ((npos != start) && (npos != end) && ((start + 1) < end)) {
            length = end - start - 1;
            if (length < out.size()) {
                std::copy_n(in.data() + start + 1, length, out.begin());
                out[length] = '\0';
            } else {
                length = 0U;
            }
        }
        return length + 2;
    }
};

} // namespace Utils
} // namespace ATL_NS