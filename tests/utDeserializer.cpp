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

#include "atlink/utils/Deserializer.h"

#include <catch2/catch_all.hpp>

SCENARIO("Tag can be deserialized") {

    using atlink::Core::Tag;
    const Tag tag{"+CSQNA"};

    GIVEN("A valid tag") {
        WHEN("Deserialized") {
            THEN("The correct number of bytes is reported") {
                atlink::Utils::Deserializer deserializer{"+CSQNA"};
                deserializer.visit(tag);
                REQUIRE(deserializer.isValid());
                REQUIRE(6U == deserializer.numberOfBytesConsumed());
            }
        }
    }

    GIVEN("An invalid tag") {
        WHEN("Deserialized") {
            THEN("The deserializer reports invalidity") {
                atlink::Utils::Deserializer deserializer{"+CSQN"};
                deserializer.visit(tag);
                REQUIRE_FALSE(deserializer.isValid());
                REQUIRE(0U == deserializer.numberOfBytesConsumed());
            }
        }
    }

    GIVEN("A valid tag with leading whitespaces") {
        WHEN("Deserialized") {
            THEN("The correct number of bytes is reported") {
                atlink::Utils::Deserializer deserializer{"  \t+CSQNA"};
                deserializer.visit(tag);
                REQUIRE(deserializer.isValid());
                REQUIRE(9U == deserializer.numberOfBytesConsumed());
            }
        }
    }
}

// TODO: implement the remaining tests