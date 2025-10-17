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

#ifndef CPIN_H
#define CPIN_H

#include <atlink/core/Command.h>
#include <atlink/core/Enum.h>
#include <atlink/core/Response.h>

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
        READY,
        SIM_PIN,
        SIM_PUK,
        PH_SIM_PIN,
        PH_SIM_PUK,
        SIM_PIN2,
        SIM_PUK2,
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
struct ATL_NS::Core::EnumTraits<ATL_NS::Proto::Std::CpinReadResponse::Code> {
    using Code = ATL_NS::Proto::Std::CpinReadResponse::Code;
    static size_t stringify(Code value, gsl::span<char> output) {
        return 0;
    }

    static size_t parse(Code &value, const gsl::span<const char> input) {
        return 0;
    }
};

#endif // CPIN_H