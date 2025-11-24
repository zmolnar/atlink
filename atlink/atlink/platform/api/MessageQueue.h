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

#include "atlink/utils/Detector.h"

namespace ATL_NS {
namespace Platform {
namespace Api {

template <typename T, typename Backend>
class MessageQueue {

    template <class C>
    using expr_get = decltype(std::declval<C &>().get());
    static_assert(ATL_NS::Utils::is_detected_exact_v<T, expr_get, Backend>,
                  "MessageQueue Backend must implement: 'T get()'");

    // Accept any reasonable backend signature: put(T), put(const T&), or put(T&&)
    template <class C>
    using expr_put = decltype(std::declval<C &>().put(std::declval<T &&>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_put, Backend>,
                  "MessageQueue Backend must implement: 'void put(T)'");

    template <class C>
    using expr_putFront = decltype(std::declval<C &>().putFront(std::declval<T &&>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_putFront, Backend>,
                  "MessageQueue Backend must implement: 'void putFront(T)'");

  public:
    T get() {
        return impl.get();
    }
    void put(T msg) {
        impl.put(msg);
    }
    void putFront(T msg) {
        impl.putFront(msg);
    }

  private:
    Backend impl;
};

} // namespace Api
} // namespace Platform
} // namespace ATL_NS