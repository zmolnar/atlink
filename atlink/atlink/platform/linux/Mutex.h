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

// atlink/platform/linux/Mutex.h
#pragma once
#include <pthread.h>

namespace ATL_NS {
namespace Platform {
namespace Impl {
namespace Linux {

class Mutex {
  public:
    using Native = pthread_mutex_t;

    Mutex() {
        ::pthread_mutex_init(&mtx, nullptr);
    }
    ~Mutex() {
        ::pthread_mutex_destroy(&mtx);
    }

    void lock() {
        ::pthread_mutex_lock(&mtx);
    }
    bool tryLock() {
        return ::pthread_mutex_trylock(&mtx) == 0;
    }
    void unlock() {
        ::pthread_mutex_unlock(&mtx);
    }

    Native &native() {
        return mtx;
    }
    const Native &native() const {
        return mtx;
    }

  private:
    Native mtx{};
};

} // namespace Linux
} // namespace Impl
} // namespace Platform
} // namespace ATL_NS