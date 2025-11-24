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

#include <array>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "atlink/core/Types.h"
#include "atlink/utils/EnumStringConverter.h"

namespace ATL_NS {
namespace Core {

// MapProvider<T>: specialize ONLY for enums that use custom strings.
// Provide inline static constexpr std::array<Record, N> map = { ... }
// which contains the string associations.
// For numeric enums, do nothing — traits will select numeric converter.
template <typename T, typename = void>
struct MapProvider;

template <typename T, typename = void>
struct has_map : std::false_type {};

template <typename T>
struct has_map<T, std::void_t<decltype(MapProvider<T>::map)>> : std::true_type {};

template <typename T, typename Enable = void>
struct EnumTraits;

// EnumTraits specialization for enums associated with custom strings
template <typename T>
struct EnumTraits<T, std::enable_if_t<has_map<T>::value>> {
  private:
    using MapRef = decltype(MapProvider<T>::map);
    static_assert(Utils::isStrictlySortedByString(MapProvider<T>::map),
                  "Enum string map must be strictly sorted by key");

    static constexpr std::size_t N = std::tuple_size<MapRef>::value;
    inline static const Utils::EnumCustomStringConverter<T, N> converter{MapProvider<T>::map};

  public:
    static size_t stringify(T value, MutableBuffer output) {
        return converter.stringify(value, output);
    }
    static size_t parse(T &value, ReadOnlyText input) {
        return converter.parse(value, input);
    }
};

// EnumTraits specialization for enums represented as numeric strings
template <typename T>
struct EnumTraits<T, std::enable_if_t<!has_map<T>::value>> {
  private:
    inline static const Utils::EnumStringConverter<T> converter{};

  public:
    static size_t stringify(T value, MutableBuffer output) {
        return converter.stringify(value, output);
    }
    static size_t parse(T &value, ReadOnlyText input) {
        return converter.parse(value, input);
    }
};

template <typename T>
class Enum : public AEnum {
  private:
    T value{};

  public:
    constexpr Enum() = default;
    constexpr Enum(T v) : value{v} {}

    Enum &operator=(T v) {
        value = v;
        return *this;
    }

    operator T() const {
        return value;
    }

    T get() const {
        return value;
    }

    bool operator==(const Enum &other) const {
        return value == other.value;
    }
    bool operator!=(const Enum &other) const {
        return value != other.value;
    }
    bool operator==(T other) const {
        return value == other;
    }
    bool operator!=(T other) const {
        return value != other;
    }

    size_t stringify(MutableBuffer output) const override {
        return EnumTraits<T>::stringify(value, output);
    }

    size_t parse(ReadOnlyText input) override {
        return EnumTraits<T>::parse(value, input);
    }
};

template <typename T>
inline bool operator==(T lhs, const Enum<T> &rhs) {
    return lhs == static_cast<T>(rhs);
}
template <typename T>
inline bool operator!=(T lhs, const Enum<T> &rhs) {
    return lhs != static_cast<T>(rhs);
}

} // namespace Core
} // namespace ATL_NS