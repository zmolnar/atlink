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

#include <atlink/core/Types.h>

namespace ATL_NS {
namespace Core {

// Forward declaration of EnumTraits
template <typename T>
struct EnumTraits {
    static const char *toString(T value) = delete;
    static std::optional<T> fromString(const char *str) = delete;
};

template <typename T>
class Enum : public AEnum {
  private:
    T value{};

  public:
    constexpr Enum() = default;

    constexpr Enum(T value) : value{value} {}

    Enum &operator=(T newValue) {
        value = newValue;
        return *this;
    }

    // Implicit conversion to underlying enum type
    operator T() const {
        return value;
    }

    // Comparison operators
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

    const char *asStr() const override {
        return EnumTraits<T>::toString(value);
    }

    bool fromStr(const char *str) override {
        auto optValue = EnumTraits<T>::fromString(str);
        if (optValue) {
            value = *optValue;
        }
        return optValue.has_value();
    }
};

template <typename T>
bool operator==(T lhs, const Enum<T> &rhs) {
    return lhs == rhs.value();
}

template <typename T>
bool operator!=(T lhs, const Enum<T> &rhs) {
    return lhs != rhs.value();
}

} // namespace Core
} // namespace ATL_NS