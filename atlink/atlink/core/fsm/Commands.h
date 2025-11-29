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

#include "atlink/core/Command.h"
#include "atlink/core/ErrorCode.h"
#include "atlink/core/Response.h"
#include "atlink/core/ResponsePack.h"

#include "atlink/platform/Facade.h"

namespace ATL_NS {
namespace Core {
namespace Fsm {
namespace Command {

struct SendCommand {
    ErrorCode *ec;
    AResponsePack *result;
    const Core::Command *command;
    Response *response;
    Platform::Semaphore *sem;
};

} // namespace Command
} // namespace Fsm
} // namespace Core
} // namespace ATL_NS