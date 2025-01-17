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

#include <type_traits>
#include <variant>

#include <atlink/core/Packet.h>

namespace ATL_NS {

class AResponse : public APacket {
  public:
    explicit AResponse(const char *tag) : APacket{tag} {}
    virtual void accept(AInputVisitor &visitor) = 0;
    virtual ~AResponse() = default;
};

} // namespace ATL_NS
