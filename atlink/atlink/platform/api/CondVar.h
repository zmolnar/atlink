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

#include "atlink/platform/api/Mutex.h"
#include "atlink/utils/Detector.h"

#include <chrono>
#include <utility>

namespace ATL_NS {
namespace Platform {
namespace Api {

template <typename Backend, typename MutexBackend>
class CondVar {
  public:
    using Duration = std::chrono::steady_clock::duration;

  private:
    using NativeMutex = typename MutexBackend::Native;

    // --- Compile-time backend checks (single-parameter detectors) ---
    template <class B>
    using expr_wait = decltype(std::declval<B &>().wait(std::declval<NativeMutex &>()));
    static_assert(Utils::is_detected_exact_v<void, expr_wait, Backend>,
                  "CondVar Backend must provide: void wait(Mutex<...>::Native&)");

    template <class B>
    using expr_waitFor = decltype(std::declval<B &>().waitFor(std::declval<NativeMutex &>(),
                                                              std::declval<Duration>()));
    static_assert(Utils::is_detected_exact_v<bool, expr_waitFor, Backend>,
                  "CondVar Backend must provide: bool waitFor(Mutex<...>::Native&, duration)");

    template <class B>
    using expr_notifyOne = decltype(std::declval<B &>().notifyOne());
    static_assert(Utils::is_detected_exact_v<void, expr_notifyOne, Backend>,
                  "CondVar Backend must provide: void notifyOne()");

    template <class B>
    using expr_notifyAll = decltype(std::declval<B &>().notifyAll());
    static_assert(Utils::is_detected_exact_v<void, expr_notifyAll, Backend>,
                  "CondVar Backend must provide: void notifyAll()");

  public:
    template <class... Args>
    explicit CondVar(Args &&...args) : impl(std::forward<Args>(args)...) {}

    void notifyOne() {
        impl.notifyOne();
    }
    void notifyAll() {
        impl.notifyAll();
    }

    void wait(Mutex<MutexBackend> &mtx) {
        impl.wait(mtx.native());
    }

    bool waitFor(Mutex<MutexBackend> &mtx, Duration dur) {
        return impl.waitFor(mtx.native(), dur);
    }

  private:
    Backend impl;
};

} // namespace Api
} // namespace Platform
} // namespace ATL_NS