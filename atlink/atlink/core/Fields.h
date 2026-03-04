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

#include "atlink/core/BasicTypes.h"
#include "atlink/core/TextBuilder.h"

namespace ATL_NS {
namespace Core {

namespace Constants {
namespace Literals {
inline constexpr Core::ReadOnlyText CrLf{"\r\n"};
inline constexpr Core::ReadOnlyText Cr{"\r"};
inline constexpr Core::ReadOnlyText Comma{","};
} // namespace Literals
} // namespace Constants

class AField {
  public:
    virtual size_t stringify(MutableBuffer output) const = 0;
    virtual size_t parse(ReadOnlyText input) = 0;
    virtual ~AField() = default;
};

class Literal : public AField {
    const ReadOnlyText text;

  public:
    explicit Literal(ReadOnlyText text) : text{text} {}

    size_t stringify(MutableBuffer output) const override {
        size_t n = 0U;
        if (text.size() < output.size()) {
            std::copy_n(text.data(), text.size(), output.data());
            n = text.size();
        }
        return n;
    }

    size_t parse(ReadOnlyText input) override {
        auto start = input.find(text);
        size_t n = 0U;
        if (0U == start) {
            n = text.size();
        }
        return n;
    }

    ReadOnlyText view() const {
        return text;
    }

    size_t length() const {
        return text.size();
    }
};

template <std::size_t N>
class LineString : public AField {
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
        auto pos = input.find(Constants::Literals::CrLf);
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
class QuotedString : public AField {
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

namespace Constants {

inline Core::Literal Cr{Literals::Cr};
inline Core::Literal CrLf{Literals::CrLf};
inline Core::Literal Comma{Literals::Comma};

} // namespace Constants
} // namespace Core
} // namespace ATL_NS
