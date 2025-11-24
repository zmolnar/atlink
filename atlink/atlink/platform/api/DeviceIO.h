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

#include <gsl/span>
#include <string_view>

namespace ATL_NS {
namespace Platform {
namespace Api {

class Subscriber {
  public:
    enum class Event {
        RxReady,
        TxReady,
    };
    virtual void notify(Event ev) = 0;
    virtual ~Subscriber() = default;
};

template <typename Backend>
class DeviceIO {

    template <class T>
    using expr_subscribe = decltype(std::declval<T &>().subscribe(std::declval<Subscriber &>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<void, expr_subscribe, Backend>,
                  "DeviceIO Backend must privide: 'void subscribe(Subscriber&)'");

    template <class T>
    using expr_write = decltype(std::declval<T &>().write(std::declval<std::string_view>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<size_t, expr_write, Backend>,
                  "DeviceIO Backend must privide: 'size_t write(std::string_view)'");

    template <class T>
    using expr_read = decltype(std::declval<T &>().read(std::declval<gsl::span<char>>()));
    static_assert(ATL_NS::Utils::is_detected_exact_v<size_t, expr_read, Backend>,
                  "DeviceIO Backend must privide: 'size_t read(gsl::span<char>)'");

  private:
    Backend impl;

  public:
    void subscribe(Subscriber &listener) {
        impl.subscribe(listener);
    }

    size_t write(std::string_view s) {
        return impl.write(s);
    }

    size_t read(gsl::span<char> buf) {
        return impl.read(buf);
    }
};

} // namespace Api
} // namespace Platform
} // namespace ATL_NS