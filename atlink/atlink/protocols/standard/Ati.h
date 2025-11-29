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
#include "atlink/core/Response.h"

namespace ATL_NS {
namespace Proto {
namespace Std {
namespace Ati {
namespace Write {

class Command : public Core::Command {
  public:
    Command() : Core::Command("ATI") {}
    bool accept(Core::ACommandVisitor &visitor) const override {
        return Core::Command::acceptImpl(visitor);
    }
};

class Response : public Core::MultiLineResponse {
  public:
    class Manufacturer : Core::Line {
      public:
        std::array<char, 32U> storage{};
        Core::LineText name{storage};

        Manufacturer() : Core::Line() {}
        bool accept(Core::AResponseVisitor &visitor) override {
            return Core::Line::acceptImpl(visitor, name);
        }
    };
    Response() : Core::MultiLineResponse("+ATI:") {}

    Manufacturer manufacturer{};
    bool accept(Core::AResponseVisitor &visitor) override {
        return Core::MultiLineResponse::acceptImpl(visitor, manufacturer);
    }
};

} // namespace Write
} // namespace Ati
} // namespace Std
} // namespace Proto
} // namespace ATL_NS