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

#include "atlink/core/fsm/IdleFwd.h"
#include "atlink/core/fsm/SendCommandFwd.h"

namespace ATL_NS {
namespace Core {
namespace Fsm {
namespace State {

inline Variant Idle::handle(Event event) {
    switch (event) {
    case Event::RxReady: {
        ctx->dispatchUrcs();
        break;
    }
    case Event::TxReady: {
        break;
    }
    default: {
        logger.error() << "unknown event: " << static_cast<int>(event);
    }
    }
    return *this;
}

inline Variant Idle::process(const Command::SendCommand &cmd) {
    return SendCommand{ctx, cmd}.start();
}

} // namespace State
} // namespace Fsm
} // namespace Core
} // namespace ATL_NS
