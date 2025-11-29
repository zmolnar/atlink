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

#include "atlink/core/Command.h"
#include "atlink/utils/Serializer.h"

#include <catch2/catch_all.hpp>
#include <iostream>
#include <string>

namespace {

class TestCommand : public ATL_NS::Core::Command {
  public:
    enum class IntEnum { Zero = 0U, One, Two, Three, Four };
    enum class StrEnum { Five, Six, Seven, Eight, Nine };

    int num{123456};
    ATL_NS::Core::QuotedField<32U> str{};
    ATL_NS::Core::Enum<IntEnum> intEnum{};
    ATL_NS::Core::Enum<StrEnum> strEnum{};

    TestCommand() : ATL_NS::Core::Command("+TEST CMD:") {}
    bool accept(ATL_NS::Core::ACommandVisitor &visitor) const override {
        return APacket::accept(visitor, num, str.view(), intEnum, strEnum);
    }
};

} // namespace

template <>
struct ATL_NS::Core::MapProvider<TestCommand::StrEnum> {
    using Enum = TestCommand::StrEnum;
    using Record = Utils::EnumCustomStringRecord<Enum>;
    inline static constexpr std::array map = {
        Record{"Eight", Enum::Eight},
        Record{"Five", Enum::Five},
        Record{"Nine", Enum::Nine},
        Record{"Seven", Enum::Seven},
        Record{"Six", Enum::Six},
    };
};

SCENARIO("Command can accept visitor") {

    GIVEN("A command") {
        auto cmd = TestCommand{};
        cmd.str.stream() << "test \"string\"";
        cmd.intEnum = TestCommand::IntEnum::Two;
        cmd.strEnum = TestCommand::StrEnum::Seven;
        WHEN("Serialized") {
            char buf[64U];
            auto serializer = ATL_NS::Utils::Serializer{buf};
            cmd.accept(serializer);
            THEN("The resulting string is as expected") {
                std::string expected{"+TEST CMD:123456,\"test \\\"string\\\"\",2,Seven\r\n"};
                REQUIRE(expected == buf);
                REQUIRE(44U == serializer.written());
            }
        }
    }
}