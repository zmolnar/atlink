// atlink/platform/linux/CondVar.h  (used for “Linux” impl; works on macOS too)
#pragma once
#include "atlink/platform/linux/Mutex.h"
#include <chrono>
#include <pthread.h>
#include <time.h>

namespace ATL_NS {
namespace Platform {
namespace Impl {
namespace Linux {

class CondVar {
  public:
    CondVar() {
        ::pthread_condattr_init(&attr_);

#if defined(__linux__)
        // Linux supports selecting the condvar clock
        ::pthread_condattr_setclock(&attr_, CLOCK_MONOTONIC);
        clock_id_ = CLOCK_MONOTONIC;
        ::pthread_cond_init(&cv_, &attr_);
#else
        // macOS / BSD: no pthread_condattr_setclock
        clock_id_ = CLOCK_REALTIME;
        ::pthread_cond_init(&cv_, &attr_);
#endif
    }

    ~CondVar() {
        ::pthread_cond_destroy(&cv_);
        ::pthread_condattr_destroy(&attr_);
    }

    // Match the native mutex type of the pthread-based Mutex backend
    void wait(Linux::Mutex::Native &mtx) {
        // Caller must hold mtx locked.
        ::pthread_cond_wait(&cv_, &mtx);
        // Returns with mtx re-locked.
    }

    bool waitFor(Linux::Mutex::Native &mtx, std::chrono::steady_clock::duration dur) {
#if defined(__APPLE__)
        // Use Apple's relative wait to avoid wall-clock jumps.
        timespec rel{};
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
        rel.tv_sec = static_cast<time_t>(ns / 1000000000LL);
        rel.tv_nsec = static_cast<long>(ns % 1000000000LL);
        int rc = ::pthread_cond_timedwait_relative_np(&cv_, &mtx, &rel);
        return rc == 0; // true if no timeout
#else
        // Linux: absolute timeout on the selected clock (monotonic as set above)
        timespec now{};
        ::clock_gettime(clock_id_, &now);

        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
        timespec abs = now;
        abs.tv_sec += static_cast<time_t>(ns / 1000000000LL);
        long nsec = static_cast<long>(ns % 1000000000LL);
        abs.tv_nsec += nsec;
        if (abs.tv_nsec >= 1000000000L) {
            abs.tv_sec += 1;
            abs.tv_nsec -= 1000000000L;
        }

        int rc = ::pthread_cond_timedwait(&cv_, &mtx, &abs);
        return rc == 0; // true if no timeout
#endif
    }

    void notifyOne() {
        ::pthread_cond_signal(&cv_);
    }
    void notifyAll() {
        ::pthread_cond_broadcast(&cv_);
    }

  private:
    pthread_cond_t cv_{};
    pthread_condattr_t attr_{};
    clockid_t clock_id_{CLOCK_REALTIME};
};

} // namespace Linux
} // namespace Impl
} // namespace Platform
} // namespace ATL_NS