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

#include "atlink/core/Enum.h"
#include "atlink/core/Response.h"

namespace ATL_NS {
namespace Proto {
namespace Std {

class CmsError : public Core::AResponse {
  public:
    enum class Code {
        Unknown = 500,
    };

    Core::Enum<Code> code{};

    CmsError() : Core::AResponse("+CMS ERROR:") {}
    ~CmsError() = default;
    bool accept(Core::AInputVisitor &visitor) override {
        return APacket::accept(visitor, code);
    }
};

} // namespace Std
} // namespace Proto
} // namespace ATL_NS