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
#include "atlink/platform/Registry.h"
#include "atlink/platform/api/CondVar.h"
#include "atlink/platform/api/DeviceIO.h"
#include "atlink/platform/api/Logger.h"
#include "atlink/platform/api/MessageQueue.h"
#include "atlink/platform/api/Mutex.h"
#include "atlink/platform/api/Semaphore.h"
#include "atlink/platform/api/Timer.h"

namespace ATL_NS {
namespace Platform {

#if defined(AT_PLATFORM_LINUX)
using ActiveTag = Tags::Linux;
#else
#error "no platform tag"
#endif

using CondVar = Api::CondVar<typename Components<ActiveTag>::CondVarImpl,
                             typename Components<ActiveTag>::MutexImpl>;

using DeviceIO = Api::DeviceIO<typename Components<ActiveTag>::DeviceIOImpl>;

using Logger = Api::Logger<typename Components<ActiveTag>::LoggerImpl>;

template <class T>
using MessageQueue =
    Api::MessageQueue<T, typename Components<ActiveTag>::template MessageQueueImpl<T>>;

using Mutex = Api::Mutex<Components<ActiveTag>::MutexImpl>;

using Semaphore = Api::Semaphore<Components<ActiveTag>::SemaphoreImpl>;

using Timer = Api::Timer<typename Components<ActiveTag>::TimerImpl>;

} // namespace Platform
} // namespace ATL_NS