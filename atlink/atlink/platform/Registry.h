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

#include "atlink/platform/Tags.h"
#include "atlink/platform/linux/CondVar.h"
#include "atlink/platform/linux/DeviceIO.h"
#include "atlink/platform/linux/Logger.h"
#include "atlink/platform/linux/MessageQueue.h"
#include "atlink/platform/linux/Mutex.h"
#include "atlink/platform/linux/Semaphore.h"
#include "atlink/platform/linux/Timer.h"

namespace ATL_NS {
namespace Platform {

template <class Tag>
struct Components;

template <>
struct Components<Tags::Linux> {
    using CondVarImpl = Impl::Linux::CondVar;
    using DeviceIOImpl = Impl::Linux::DeviceIO;
    using LoggerImpl = Impl::Linux::Logger;
    template <class T>
    using MessageQueueImpl = Impl::Linux::MessageQueue<T>;
    using MutexImpl = Impl::Linux::Mutex;
    using SemaphoreImpl = Impl::Linux::Semaphore;
    using TimerImpl = Impl::Linux::Timer;
};

} // namespace Platform
} // namespace ATL_NS