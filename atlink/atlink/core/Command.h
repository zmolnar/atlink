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

#include <atlink/core/Packet.h>

namespace ATL_NS {
namespace Core {

class ACommand : public APacket {
  public:
    explicit ACommand(const char *tag) : APacket{tag} {};
    virtual bool accept(AOutputVisitor &visitor) const = 0;
    virtual ~ACommand() = default;

  protected:
    template <typename... Args>
    bool acceptImpl(AOutputVisitor &visitor, Args &&...args) const {
        visitor.reset();
        return APacket::acceptWithTerm(visitor,
                                       Constants::Mandatory::Cr,
                                       std::forward<Args>(args)...);
    }
};

} // namespace Core
} // namespace ATL_NS
