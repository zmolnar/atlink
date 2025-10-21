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

#include <atlink/core/Command.h>
#include <atlink/core/Enum.h>
#include <atlink/core/Response.h>
#include <atlink/utils/EnumStringConverter.h>

namespace ATL_NS {
namespace Proto {
namespace Std {

class CpinRead : public Core::ACommand {
  public:
    CpinRead() : Core::ACommand("+CPIN? ") {}
    void accept(Core::AOutputVisitor &visitor) const override {
        APacket::accept(visitor);
    }
};

class CpinWrite : public Core::ACommand {
  public:
    CpinWrite() : Core::ACommand("+CPIN=") {}
    int pin;
    void accept(Core::AOutputVisitor &visitor) const override {
        APacket::accept(visitor, pin);
    }
};

class CpinReadResponse : public Core::AResponse {
  public:
    enum class Code {
        Ready,
        SimPin,
        SimPuk,
        PhSimPin,
        PhSimPuk,
        PhSimPin2,
        PhSimPuk2,
    };

    Core::Enum<Code> code;

    CpinReadResponse() : Core::AResponse("+CPIN: ") {}
    ~CpinReadResponse() = default;
    void accept(Core::AInputVisitor &visitor) override {
        APacket::accept(visitor, code);
    }
};

} // namespace Std
} // namespace Proto
} // namespace ATL_NS


template <>
struct ATL_NS::Core::MapProvider<ATL_NS::Proto::Std::CpinReadResponse::Code> {
    using Enum = ATL_NS::Proto::Std::CpinReadResponse::Code;
    using Record = Utils::EnumCustomStringRecord<Enum>;
    inline static constexpr std::array map = {
        Record{"PH_SIM_PIN", Enum::PhSimPin},
        Record{"PH_SIM_PUK", Enum::PhSimPuk},
        Record{"READY", Enum::Ready},
        Record{"SIM_PIN", Enum::SimPin},
        Record{"SIM_PIN2", Enum::PhSimPin2},
        Record{"SIM_PUK", Enum::SimPuk},
        Record{"SIM_PUK2", Enum::PhSimPuk2},
    };
};