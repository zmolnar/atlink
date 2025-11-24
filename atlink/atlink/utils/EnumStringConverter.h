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

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <magic_enum/magic_enum.hpp>
#include <optional>

#include "atlink/core/Types.h"

namespace ATL_NS {
namespace Utils {

template <typename T, size_t N = 0U>
class EnumStringConverter {
  public:
    using ReadOnlyText = ATL_NS::Core::ReadOnlyText;
    using MutableBuffer = ATL_NS::Core::MutableBuffer;

    constexpr EnumStringConverter() = default;

    constexpr size_t stringify(T value, MutableBuffer output) const {
        static_assert(sizeof(T) <= sizeof(int), "T must not be larger than int");
        size_t n = 0U;
        if (0U < output.size()) {
            auto *first = output.data();
            auto *last = output.data() + output.size() - 1U;
            auto num = static_cast<int>(value);
            auto result = std::to_chars(first, last, num);
            if (result.ec == std::errc{}) {
                n = result.ptr - output.data();
                output[n] = '\0';
            }
        }
        return n;
    }

    constexpr size_t parse(T &value, ReadOnlyText input) const {
        static_assert(sizeof(T) <= sizeof(int), "T must not be larger than int");

        const auto *first = input.data();
        const auto *last = input.data() + input.size();
        int num = 0U;
        auto result = std::from_chars(first, last, num);

        size_t n = 0U;
        if (result.ec == std::errc{}) {
            auto opt_enum = magic_enum::enum_cast<T>(num);
            if (opt_enum.has_value()) {
                value = opt_enum.value();
                n = result.ptr - input.data();
            }
        }
        return n;
    }
};

template <typename T, size_t N>
class EnumCustomStringConverter {
  public:
    using ReadOnlyText = ATL_NS::Core::ReadOnlyText;
    using MutableBuffer = ATL_NS::Core::MutableBuffer;
    using Record = std::pair<ATL_NS::Core::ReadOnlyText, T>;
    using Map = std::array<Record, N>;

    constexpr explicit EnumCustomStringConverter(const Map &map) : map{map} {}

    constexpr size_t stringify(T value, MutableBuffer output) const {
        size_t n = 0;
        auto str = lookup(value);
        if (str.size() < output.size()) {
            n = str.size();
            std::copy_n(str.data(), str.size(), output.data());
        }
        return n;
    }

    constexpr size_t parse(T &value, ReadOnlyText input) const {
        size_t n = 0U;
        auto result = lookup(input);
        if (result) {
            value = result.value();
            n = lookup(value).size();
        }
        return n;
    }

  private:
    const Map &map;

    // Lookup for the given variant in the association map.
    // The complexity is O(n) but is it acceptable, because
    // enum to string conversion is much less frequent than
    // the opposite direction. Can be improved later.
    constexpr ReadOnlyText lookup(T variant) const {
        auto matcher = [variant](const Record &i) {
            return i.second == variant;
        };
        auto it = std::find_if(map.begin(), map.end(), matcher);
        assert(it != map.end());
        return it->first;
    }

    // Lookup for the given string in the association map.
    // The complexity of this search is O(log n)
    constexpr std::optional<T> lookup(ReadOnlyText str) const {
        if (str.empty())
            return std::nullopt;

        auto comp = [](const Record &item, std::string_view value) {
            return (item.first < (value.substr(0, item.first.size())));
        };

        auto it = std::lower_bound(map.begin(), map.end(), str, comp);

        if ((it != map.end()) && (it->first.size() <= str.size()) &&
            (str.substr(0, it->first.size()) == it->first)) {
            return it->second;
        }

        return std::nullopt;
    }
};

template <typename T>
using EnumCustomStringRecord = typename EnumCustomStringConverter<T, 1U>::Record;

template <typename T, size_t N>
EnumCustomStringConverter(const std::array<std::pair<std::string_view, T>, N> &)
    -> EnumCustomStringConverter<T, N>;

template <typename T, size_t N>
constexpr bool isStrictlySortedByString(const std::array<std::pair<std::string_view, T>, N> &arr) {
    if constexpr (N <= 1)
        return true;

    for (size_t i = 1; i < N; ++i) {
        if (arr[i - 1].first >= arr[i].first) {
            return false;
        }
    }
    return true;
}

} // namespace Utils
} // namespace ATL_NS