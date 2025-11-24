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

#include <cassert>
#include <cerrno>
#include <semaphore.h>

namespace ATL_NS {
namespace Platform {
namespace Impl {
namespace Linux {

class Semaphore {
  public:
    explicit Semaphore(uint32_t initial = 0) {
        int rc = ::sem_init(&sem, 0, initial);
        assert(rc == 0 && "sem_init failed");
        (void)rc;
    }

    ~Semaphore() {
        ::sem_destroy(&sem);
    }

    // Non-copyable
    Semaphore(const Semaphore &) = delete;
    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore(Semaphore &&) = delete;
    Semaphore &operator=(Semaphore &&) = delete;

    void acquire() {
        while (true) {
            if (::sem_wait(&sem) == 0)
                return;
            if (errno == EINTR)
                continue; // retry on signal
            assert(false && "sem_wait failed");
        }
    }

    void release() {
        int rc = ::sem_post(&sem);
        assert(rc == 0 && "sem_post failed");
        (void)rc;
    }

  private:
    sem_t sem{};
};

} // namespace Linux
} // namespace Impl
} // namespace Platform
} // namespace ATL_NS