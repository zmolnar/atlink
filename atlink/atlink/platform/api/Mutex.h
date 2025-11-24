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
class Mutex {
  public:
    using Native = typename Backend::Native;

    template <class T>
    using expr_lock = decltype(std::declval<T &>().lock());
    static_assert(Utils::is_detected_exact_v<void, expr_lock, Backend>,
                  "Mutex Backend must provide: void lock()");

    template <class T>
    using expr_tryLock = decltype(std::declval<T &>().tryLock());
    static_assert(Utils::is_detected_exact_v<bool, expr_tryLock, Backend>,
                  "Mutex Backend must provide: bool tryLock()");

    template <class T>
    using expr_unlock = decltype(std::declval<T &>().unlock());
    static_assert(Utils::is_detected_exact_v<void, expr_unlock, Backend>,
                  "Mutex Backend must provide: void unlock()");

    template <typename T>
    using expr_native = decltype(std::declval<T &>().native());
    static_assert(Utils::is_detected_exact_v<Native &, expr_native, Backend>,
                  "Mutex Backend must provide: Native& native()");

    template <class... Args>
    explicit Mutex(Args &&...args) : impl(std::forward<Args>(args)...) {}

    Native &native() {
        return impl.native();
    }

    const Native &native() const {
        return impl.native();
    }

    void lock() {
        impl.lock();
    }

    bool tryLock() {
        return impl.tryLock();
    }

    void unlock() {
        impl.unlock();
    }

    class LockGuard {
        Mutex &mtx;

      public:
        explicit LockGuard(Mutex &mtx) : mtx{mtx} {
            mtx.lock();
        }
        ~LockGuard() {
            mtx.unlock();
        }
    };

  private:
    Backend impl;
};

} // namespace Api
} // namespace Platform
} // namespace ATL_NS