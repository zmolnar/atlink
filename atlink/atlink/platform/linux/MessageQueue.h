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

#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

namespace ATL_NS {
namespace Platform {
namespace Impl {
namespace Linux {

template <typename T>
class MessageQueue {
  public:
    MessageQueue() = default;

    void put(T msg) {
        {
            std::lock_guard<std::mutex> lk(mutex);
            queue.push_back(std::move(msg));
        }
        condvar.notify_one();
    }

    void putFront(T msg) {
        {
            std::lock_guard<std::mutex> lk(mutex);
            queue.push_front(std::move(msg));
        }
        condvar.notify_one();
    }

    T get() {
        std::unique_lock<std::mutex> lk(mutex);
        condvar.wait(lk, [this] {
            return !queue.empty();
        });
        T v = std::move(queue.front());
        queue.pop_front();
        return v;
    }

  private:
    mutable std::mutex mutex;
    std::condition_variable condvar;
    std::deque<T> queue;
};

} // namespace Linux
} // namespace Impl
} // namespace Platform
} // namespace ATL_NS
