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

#include "atlink/core/Types.h"

namespace ATL_NS {
namespace Core {
namespace Fsm {

class Context {
  public:
    virtual bool send(const Core::Command &out) = 0;
    virtual bool receive(AResponsePack &frc, Response *in) = 0;
    virtual bool canSend() = 0;
    virtual void dispatchUrcs() = 0;
    virtual ~Context() = default;
};

} // namespace Fsm
} // namespace Core
} // namespace ATL_NS