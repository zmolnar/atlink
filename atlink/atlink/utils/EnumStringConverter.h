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
#include <optional>
#include <string_view>

namespace ATL_NS {
namespace Utils {

template <typename T, size_t N>
class EnumStringConverter {
  public:
    using Record = std::pair<std::string_view, T>;
    using Map = std::array<Record, N>;

    constexpr EnumStringConverter(const Map &map) : map{map} {}

    constexpr std::string_view toString(T variant) const {
        auto matcher = [variant](const Record &i) {
            return i.second == variant;
        };
        auto it = std::find_if(map.begin(), map.end(), matcher);
        assert(it != map.end());
        return it->first;
    }

    constexpr std::optional<T> fromString(std::string_view str) const {
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

  private:
    const Map &map;
};

// Helper alias to access Record type with just T
template <typename T>
using EnumStringRecord = typename EnumStringConverter<T, 1>::Record;

// Deduction guide
template <typename T, size_t N>
EnumStringConverter(const std::array<std::pair<std::string_view, T>, N> &)
    -> EnumStringConverter<T, N>;

template <typename T, size_t N>
constexpr bool isStrictlySortedByString(
    const std::array<std::pair<std::string_view, T>, N> &arr) {
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