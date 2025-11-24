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

#include <type_traits>
#include <utility>
#include <variant>

#include "atlink/core/Response.h"

namespace ATL_NS {
namespace Core {

class AResponsePack {
  public:
    virtual bool accept(AInputVisitor &visitor) = 0;
    virtual ~AResponsePack() = default;
};

template <typename... Rs>
class ResponsePack : public AResponsePack {
    static_assert(sizeof...(Rs) > 0, "ResponsePack requires at least one response type");
    static_assert((std::is_base_of<AResponse, Rs>::value && ...),
                  "All Rs must derive from ATL_NS::Core::AResponse");
    static_assert((std::is_default_constructible<Rs>::value && ...),
                  "All Rs must be default-constructible for trial parsing");

  public:
    using Variant = std::variant<std::monostate, Rs...>;

    ResponsePack() = default;

    void reset() {
        value = std::monostate{};
    }

    // Try to parse with each alternative in order.
    // On success, stores the parsed object and returns true.
    bool accept(AInputVisitor &visitor) override {
        return tryAll<0, Rs...>(visitor);
    }

    const Variant &getValue() const noexcept {
        return value;
    }
    Variant &getValue() noexcept {
        return value;
    }

    template <typename T>
    bool holds() const noexcept {
        return std::holds_alternative<T>(value);
    }

    template <typename T>
    const T *getIf() const noexcept {
        return std::get_if<T>(&value);
    }

    template <typename T>
    T *getIf() noexcept {
        return std::get_if<T>(&value);
    }

  private:
    template <typename T>
    bool tryOne(AInputVisitor &visitor) {
        static_assert(std::is_base_of<AResponse, T>::value, "T must derive from AResponse");
        T candidate{};
        auto match = candidate.accept(visitor);
        if (match) {
            value.template emplace<T>(std::move(candidate));
        }
        return match;
    }

    template <std::size_t, typename First>
    bool tryAll(AInputVisitor &visitor) {
        return tryOne<First>(visitor);
    }

    template <std::size_t Index, typename First, typename Second, typename... Rest>
    bool tryAll(AInputVisitor &visitor) {
        if (tryOne<First>(visitor)) {
            return true;
        }
        return tryAll<Index + 1, Second, Rest...>(visitor);
    }

  private:
    Variant value;
};

} // namespace Core
} // namespace ATL_NS