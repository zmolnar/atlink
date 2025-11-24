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

namespace ATL_NS {
namespace Utils {

template <class...>
using void_t = void;

struct none {
    none() = delete;
    ~none() = delete;
    none(none const &) = delete;
    void operator=(none const &) = delete;
};

template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

template <template <class...> class Op, class... Args>
using is_detected = typename detector<none, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename detector<none, void, Op, Args...>::type;

// Exact-type check: Expected, Op<Args...>
template <class Expected, template <class...> class Op, class... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

template <class Expected, template <class...> class Op, class... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

} // namespace Utils
} // namespace ATL_NS