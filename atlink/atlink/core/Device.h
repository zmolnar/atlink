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

#include "atlink/core/fsm/Orchestrator.h"
#include "atlink/platform/Facade.h"

namespace ATL_NS {
namespace Core {

class Device {

    Platform::Logger logger;
    Fsm::Orchestrator orchestrator;

  public:
    Device(const char *name, Platform::DeviceIO &io, AUrcDispatcher &udp)
        : logger{name}, orchestrator{io, udp} {}

    void loop() {
        orchestrator.loop();
    }

    bool sendCommand(AResponsePack *result, Command *cmd, Response *res) {
        auto ec = orchestrator.sendCommand(result, cmd, res);
        return (ErrorCode::NoError == ec);
    }

    void shutDown() {
        orchestrator.shutDown();
    }
};

} // namespace Core
} // namespace ATL_NS