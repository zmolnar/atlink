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

#include <climits>
#include <cstddef>
#include <semaphore>

namespace ATL_NS {
namespace Platform {
namespace Impl {
namespace Linux {

class Semaphore {
  public:
    explicit Semaphore(uint32_t initial = 0) : sem{static_cast<std::ptrdiff_t>(initial)} {}

    ~Semaphore() = default;

    // Non-copyable
    Semaphore(const Semaphore &) = delete;
    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore(Semaphore &&) = delete;
    Semaphore &operator=(Semaphore &&) = delete;

    void acquire() {
        sem.acquire();
    }

    void release() {
        sem.release();
    }

  private:
    std::counting_semaphore<INT_MAX> sem;
};

} // namespace Linux
} // namespace Impl
} // namespace Platform
} // namespace ATL_NS