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

#include "atlink/core/Response.h"
#include "atlink/utils/Deserializer.h"

#include <catch2/catch_all.hpp>
#include <iostream>

namespace {

class TestResponse : public ATL_NS::Core::AResponse {
  public:
    enum class IntEnum { Zero = 0U, One, Two, Three, Four };
    enum class StrEnum { Five, Six, Seven, Eight, Nine };

    int num;
    std::array<char,32U> str {};
    ATL_NS::Core::QuotedString strBuf{str};
    ATL_NS::Core::Enum<IntEnum> intEnum{};
    ATL_NS::Core::Enum<StrEnum> strEnum{};
    TestResponse() : ATL_NS::Core::AResponse("+TEST:") {}
    bool accept(ATL_NS::Core::AInputVisitor &visitor) {
        return AResponse::acceptImpl(visitor, num, strBuf, intEnum, strEnum);
    }
};

} // namespace

template <>
struct ATL_NS::Core::MapProvider<TestResponse::StrEnum> {
    using Enum = TestResponse::StrEnum;
    using Record = Utils::EnumCustomStringRecord<Enum>;
    inline static constexpr std::array map = {
        Record{"Eight", Enum::Eight},
        Record{"Five", Enum::Five},
        Record{"Nine", Enum::Nine},
        Record{"Seven", Enum::Seven},
        Record{"Six", Enum::Six},
    };
};

SCENARIO("Response can accept visitor") {

    GIVEN("A response") {
        auto testResponse = TestResponse{};
        WHEN("A valid input string is visited") {
            atlink::Utils::Deserializer deserializer{"+TEST: 322, \"input string\",   4, Five   \r\n"};
            auto success = testResponse.accept(deserializer);
            THEN("The proper values are deserialized") {
                REQUIRE(success);
                REQUIRE(42U == deserializer.numberOfBytesConsumed());
                REQUIRE(322 == testResponse.num);
                REQUIRE(0 == strcmp("input string", testResponse.str.data()));
                REQUIRE(TestResponse::IntEnum::Four == testResponse.intEnum.get());
                REQUIRE(TestResponse::StrEnum::Five == testResponse.strEnum.get());
            }
        }

        WHEN("A valid input string with leading CRLF is visited") {
            atlink::Utils::Deserializer deserializer{"\r\n+TEST: 322, \"input string\",   4, Five   \r\n"};
            auto success = testResponse.accept(deserializer);
            THEN("The proper values are deserialized") {
                REQUIRE(success);
                REQUIRE(44U == deserializer.numberOfBytesConsumed());
                REQUIRE(322 == testResponse.num);
                REQUIRE(0 == strcmp("input string", testResponse.str.data()));
                REQUIRE(TestResponse::IntEnum::Four == testResponse.intEnum.get());
                REQUIRE(TestResponse::StrEnum::Five == testResponse.strEnum.get());
            }
        }
    }
}
