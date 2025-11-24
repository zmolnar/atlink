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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace ATL_NS {
namespace Platform {
namespace Impl {
namespace Linux {

class Timer {
  public:
    using Callback = void (*)(void *);
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;

    Timer() = default;

    ~Timer() {
        stop();
    }

    void setHandler(Callback cb, void *user) {
        std::lock_guard<std::mutex> lk(mutex);
        callback = cb;
        context = user;
    }

    void start(Duration d) {
        stop();
        {
            std::lock_guard<std::mutex> lk(mutex);
            stopFlag = false;
            running.store(true, std::memory_order_release);
            ++generation; // bump generation to invalidate old waits
        }
        const auto gen = generation.load(std::memory_order_acquire);

        worker = std::thread([this, d, gen]() {
            std::unique_lock<std::mutex> lk(mutex);
            // Wait until timeout or canceled/restarted (generation change)
            bool canceled = condvar.wait_for(lk, d, [this, gen] {
                return stopFlag || gen != generation.load(std::memory_order_acquire);
            });
            if (!canceled) {
                // Fire outside the lock to avoid handler-induced deadlocks
                auto cb = callback;
                auto ctx = context;
                lk.unlock();
                if (cb)
                    cb(ctx);
                lk.lock();
            }
            running.store(false, std::memory_order_release);
        });
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lk(mutex);
            if (!running.load(std::memory_order_acquire)) {
                // nothing to do
            } else {
                stopFlag = true;
                ++generation;
                condvar.notify_all();
            }
        }
        if (worker.joinable())
            worker.join();
        running.store(false, std::memory_order_release);
    }

    bool isRunning() const {
        return running.load(std::memory_order_acquire);
    }

    // Non-copyable
    Timer(const Timer &) = delete;
    Timer &operator=(const Timer &) = delete;

  private:
    Callback callback{nullptr};
    void *context{nullptr};
    bool stopFlag{false};

    // generation helps cancel/restart without racing the waiter
    std::atomic<uint64_t> generation{0};

    std::atomic<bool> running{false};
    std::thread worker;
    mutable std::mutex mutex;
    std::condition_variable condvar;
};

} // namespace Linux
} // namespace Impl
} // namespace Platform
} // namespace ATL_NS