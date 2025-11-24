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
#include <utility>

namespace ATL_NS {
namespace Platform {
namespace Api {

template <class Backend>
class Semaphore {
  private:
    // ---- Backend detectors (no macros) ----
    template <class T>
    using expr_acquire = decltype(std::declval<T &>().acquire());
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_acquire, Backend>,
                  "Semaphore Backend must provide: void acquire()");

    template <class T>
    using expr_release = decltype(std::declval<T &>().release());
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_release, Backend>,
                  "Semaphore Backend must provide: void release()");

  public:
    template <class... Args>
    explicit Semaphore(Args &&...args) : impl{std::forward<Args>(args)...} {}

    void acquire() {
        impl.acquire();
    }

    void release() {
        impl.release();
    }

  private:
    Backend impl;
};

} // namespace Api
} // namespace Platform
} // namespace ATL_NS