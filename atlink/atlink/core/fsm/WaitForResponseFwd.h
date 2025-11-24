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

#include "atlink/core/fsm/Context.h"
#include "atlink/core/fsm/Events.h"
#include "atlink/core/fsm/StateFwd.h"

#include "atlink/platform/Facade.h"

namespace ATL_NS {
namespace Core {
namespace Fsm {
namespace State {

class WaitForResponse {
    Context *ctx;
    Command::SendCommand msg;
    static inline Platform::Logger logger{"FSM: state-wait-for-response"};

  public:
    WaitForResponse(Context *ctx, Command::SendCommand msg) : ctx{ctx}, msg{msg} {}
    WaitForResponse(const WaitForResponse &) = default;
    WaitForResponse &operator=(const WaitForResponse &) = default;

    Variant handle(const Event &);
    Variant onEnter();
};

} // namespace State
} // namespace Fsm
} // namespace Core
} // namespace ATL_NS
