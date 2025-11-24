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
#include <chrono>
#include <utility>

namespace ATL_NS {
namespace Platform {
namespace Api {

template <class Backend>
class Timer {
  public:
    using Callback = void (*)(void *ctx);
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;

  private:
    template <class T>
    using expr_setHandler =
        decltype(std::declval<T &>().setHandler(std::declval<Callback>(), std::declval<void *>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_setHandler, Backend>,
                  "Timer Backend must provide: void setHandler(Callback, void*)");

    template <class T>
    using expr_start = decltype(std::declval<T &>().start(std::declval<Duration>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_start, Backend>,
                  "Timer Backend must provide: void start(Duration)");

    template <class T>
    using expr_stop = decltype(std::declval<T &>().stop());
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_stop, Backend>,
                  "Timer Backend must provide: void stop()");

    template <class T>
    using expr_isRunning = decltype(std::declval<const T &>().isRunning());
    static_assert(ATL_NS::Utils::is_detected_exact_v<bool, expr_isRunning, Backend>,
                  "Timer Backend must provide: bool isRunning() const");

  public:
    template <class... Args>
    explicit Timer(Args &&...args) : impl(std::forward<Args>(args)...) {}

    void setHandler(Callback cb, void *user) {
        impl.setHandler(cb, user);
    }

    void start(Duration dur) {
        impl.start(dur);
    }

    void stop() {
        impl.stop();
    }

    bool isRunning() const {
        return impl.isRunning();
    }

  private:
    Backend impl;
};

} // namespace Api
} // namespace Platform
} // namespace ATL_NS