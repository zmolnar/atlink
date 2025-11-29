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

class Command : public APacket {
  public:
    explicit Command(const char *tag) : APacket{tag} {};
    virtual bool accept(ACommandVisitor &visitor) const = 0;
    virtual ~Command() = default;

  protected:
    template <typename... Args>
    bool acceptImpl(ACommandVisitor &visitor, Args &&...args) const {
        return APacket::acceptWithTerm(visitor, Constants::Cr, std::forward<Args>(args)...);
    }
};

} // namespace Core
} // namespace ATL_NS
