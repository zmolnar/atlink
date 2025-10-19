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
struct ATL_NS::Core::EnumTraits<ATL_NS::Proto::Std::CpinReadResponse::Code> {
    using Code = ATL_NS::Proto::Std::CpinReadResponse::Code;
    using Record = Utils::EnumCustomStringRecord<Code>;

    static constexpr std::array map = {
        Record{"PH_SIM_PIN", Code::PhSimPin},
        Record{"PH_SIM_PUK", Code::PhSimPuk},
        Record{"READY", Code::Ready},
        Record{"SIM_PIN", Code::SimPin},
        Record{"SIM_PIN2", Code::PhSimPin2},
        Record{"SIM_PUK", Code::SimPuk},
        Record{"SIM_PUK2", Code::PhSimPuk2},
    };

    static_assert(Utils::isStrictlySortedByString(map),
                  "CPIN response map shall be ordered by string");

    template <size_t N>
    using Converter = Utils::EnumCustomStringConverter<Code, N>;
    inline static constexpr auto converter = Converter{map};

    static size_t stringify(Code value, MutableBuffer output) {
        return converter.stringify(value, output);
    }

    static size_t parse(Code &value, ReadOnlyText input) {
        return converter.parse(value, input);
    }
};