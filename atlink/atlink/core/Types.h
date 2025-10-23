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

#include <gsl/span>
#include <string_view>

namespace ATL_NS {
namespace Core {

using ReadOnlyText = std::string_view;
using MutableBuffer = gsl::span<char>;

class Sequence {
    ReadOnlyText seq;

  public:
    explicit constexpr Sequence(ReadOnlyText seq) : seq{seq} {}

    size_t stringify(MutableBuffer output) const {
        size_t n = 0U;
        if (seq.size() < output.size()) {
            n = seq.size();
            std::copy_n(seq.data(), seq.size(), output.data());
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
};

using Tag = Sequence;

class Comma : public Sequence {
  public:
    constexpr Comma() : Sequence{","} {}
};

class Term : public Sequence {
  public:
    constexpr Term() : Sequence{"\r\n"} {}
};

class AEnum {
  public:
    virtual size_t stringify(MutableBuffer output) const = 0;
    virtual size_t parse(ReadOnlyText input) = 0;
    virtual ~AEnum() = default;
};

} // namespace Core
} // namespace ATL_NS
