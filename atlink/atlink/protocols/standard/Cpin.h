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

class CpinRead : public ATL_NS::ACommand {
  public:
    CpinRead() : ATL_NS::ACommand("+CPIN? ") {}
    void accept(ATL_NS::AOutputVisitor &visitor) const override {
        APacket::accept(visitor);
    }
};

class CpinWrite : public ATL_NS::ACommand {
  public:
    CpinWrite() : ATL_NS::ACommand("+CPIN=") {}
    int pin;
    void accept(ATL_NS::AOutputVisitor &visitor) const override {
        APacket::accept(visitor, pin);
    }
};

class CpinReadResponse : public ATL_NS::AResponse {
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

    ATL_NS::Enum<Code> code;

    CpinReadResponse() : ATL_NS::AResponse("+CPIN: ") {}
    ~CpinReadResponse() = default;
    void accept(ATL_NS::AInputVisitor &visitor) override {
        APacket::accept(visitor, code);
    }
};

} // namespace Std
} // namespace Proto
} // namespace ATL_NS

template <>
struct ATL_NS::EnumTraits<ATL_NS::Proto::Std::CpinReadResponse::Code> {
    static const char *toString(ATL_NS::Proto::Std::CpinReadResponse::Code value) {
        switch (value) {
        case ATL_NS::Proto::Std::CpinReadResponse::Code::READY:
            return "READY";
        case ATL_NS::Proto::Std::CpinReadResponse::Code::SIM_PIN:
            return "SIM PIN";
        case ATL_NS::Proto::Std::CpinReadResponse::Code::SIM_PUK:
            return "SIM PUK";
        case ATL_NS::Proto::Std::CpinReadResponse::Code::PH_SIM_PIN:
            return "PH_SIM PIN";
        case ATL_NS::Proto::Std::CpinReadResponse::Code::PH_SIM_PUK:
            return "PH_SIM PUK";
        case ATL_NS::Proto::Std::CpinReadResponse::Code::SIM_PIN2:
            return "SIM PIN2";
        case ATL_NS::Proto::Std::CpinReadResponse::Code::SIM_PUK2:
            return "SIM PUK2";
        default:
            return "";
        }
    }

    static std::optional<ATL_NS::Proto::Std::CpinReadResponse::Code> fromString(const char *str) {
        if (0 == std::strncmp(str, "READY", 5)) {
            return ATL_NS::Proto::Std::CpinReadResponse::Code::READY;
        } else if (0 == std::strncmp(str, "SIM PIN", 7)) {
            return ATL_NS::Proto::Std::CpinReadResponse::Code::SIM_PIN;
        } else if (0 == std::strncmp(str, "SIM PUK", 7)) {
            return ATL_NS::Proto::Std::CpinReadResponse::Code::SIM_PUK;
        } else if (0 == std::strncmp(str, "PH_SIM PIN", 10)) {
            return ATL_NS::Proto::Std::CpinReadResponse::Code::PH_SIM_PIN;
        } else if (0 == std::strncmp(str, "PH_SIM PUK", 10)) {
            return ATL_NS::Proto::Std::CpinReadResponse::Code::PH_SIM_PUK;
        } else if (0 == std::strncmp(str, "SIM PIN2", 8)) {
            return ATL_NS::Proto::Std::CpinReadResponse::Code::SIM_PIN2;
        } else if (0 == std::strncmp(str, "SIM PUK2", 8)) {
            return ATL_NS::Proto::Std::CpinReadResponse::Code::SIM_PUK2;
        } else {
            return std::nullopt;
        }
    }
};

#endif // CPIN_H