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

#include "atlink/core/fsm/SendCommandFwd.h"
#include "atlink/core/fsm/WaitForResponseFwd.h"

#include "atlink/utils/Serializer.h"

namespace ATL_NS {
namespace Core {
namespace Fsm {
namespace State {

inline Variant SendCommand::handle(const Event event) {
    Variant next = {*this};

    switch (event) {

    case Event::RxReady: {
        logger.debug() << "RX: URC received while sending";
        ctx->dispatchUrcs();
        break;
    }

    case Event::TxReady: {
        logger.debug() << "TX: cooldown expired → sending command";
        auto success = ctx->send(*msg.command);
        next = (success ? Variant{WaitForResponse{ctx, msg}} : Variant{Idle{ctx}});
        break;
    }

    default: {
        logger.warn() << "FSM: unhandled event (" << static_cast<int>(event) << ")";
        break;
    }
    }

    return next;
}

inline Variant SendCommand::start() {
    auto next = Variant{*this};

    if (ctx->canSend()) {
        logger.debug() << "TX: ready → sending command";
        auto success = ctx->send(*msg.command);

        if (success) {
            logger.info() << "TX: command sent";
            next = std::move(Variant{WaitForResponse{ctx, msg}});
        } else {
            logger.error() << "TX: send failed";
            next = std::move(Variant{Idle{ctx}});
        }

    } else {
        logger.debug() << "TX: cooldown active → waiting";
    }

    return next;
}

} // namespace State
} // namespace Fsm
} // namespace Core
} // namespace ATL_NS
