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

#include "atlink/utils/EnumStringConverter.h"

#include <catch2/catch_all.hpp>
#include <map>

SCENARIO("Enum to custom string converter works") {

    enum class TestEnum {
        VALUE_1,
        VALUE_2,
        VALUE_3,
        VALUE_4,
        VALUE_5,
    };

    using Record = atlink::Utils::EnumCustomStringRecord<TestEnum>;
    static constexpr std::array map = {
        Record{"value five", TestEnum::VALUE_5},
        Record{"value four", TestEnum::VALUE_4},
        Record{"value one", TestEnum::VALUE_1},
        Record{"value three", TestEnum::VALUE_3},
        Record{"value two", TestEnum::VALUE_2},
    };

    // This triggers a compile time error if the map is not ordered properly.
    static_assert(atlink::Utils::isStrictlySortedByString(map),
                  "the map shall be sorted by string");

    constexpr auto converter = atlink::Utils::EnumCustomStringConverter{map};

    GIVEN("An enum variant") {

        auto item = GENERATE(from_range(map));
        auto refStr = item.first;
        auto refVariant = item.second;

        WHEN("Converted to string") {

            std::array<char, 32U> buf{};
            auto nbytes = converter.stringify(refVariant, buf);
            auto generatedStr = std::string_view{buf.data(), nbytes};

            THEN("The associated string is returned") {
                REQUIRE(refStr == generatedStr);
            }

            THEN("The number of bytes returned is correct") {
                REQUIRE(refStr.size() == nbytes);
            }
        }
    }

    GIVEN("A valid string") {

        auto item = GENERATE(from_range(map));
        auto refStr = item.first;
        auto refVariant = item.second;

        WHEN("Converted to enum") {

            auto variant = TestEnum{};
            auto nbytes = converter.parse(variant, refStr);

            THEN("The associated variant is returned") {
                REQUIRE(refVariant == variant);
            }

            THEN("The number of bytes returned is correct") {
                REQUIRE(refStr.size() == nbytes);
            }
        }
    }

    GIVEN("Invalid strings") {

        auto str = GENERATE(std::string_view{"abc"},
                            std::string_view{"12x34"},
                            std::string_view{"this is a very long string that matches nothing"},
                            std::string_view{""});

        WHEN("Converted to enum") {

            const auto original = TestEnum::VALUE_3;
            auto variant = original;
            auto nbytes = converter.parse(variant, str);

            THEN("Returns 0 bytes") {
                REQUIRE(0U == nbytes);
            }

            THEN("Doesn't have side-effects") {
                REQUIRE(original == variant);
            }
        }
    }

    GIVEN("Buffer exactly fitting the output but no space for termination") {
        WHEN("Converting VALUE_1 (value one) to 9-char buffer") {
            std::array<char, 9U> buf{};
            auto nbytes = converter.stringify(TestEnum::VALUE_1, buf);

            THEN("Conversion fails") {
                REQUIRE(nbytes == 0U);
            }
        }
    }

    GIVEN("Zero-sized buffer") {
        WHEN("Converting any enum") {
            std::array<char, 0U> buf{};
            auto nbytes = converter.stringify(TestEnum::VALUE_1, buf);

            THEN("Returns 0") {
                REQUIRE(nbytes == 0U);
            }
        }
    }
}

SCENARIO("Enum can be mapped to numbers represented as string") {

    enum class TestEnum {
        VALUE_1 = 130,
        VALUE_2,
        VALUE_3,
        VALUE_4,
        VALUE_5,
    };

    static std::map<std::string_view, TestEnum> map = {
        {"130", TestEnum::VALUE_1},
        {"131", TestEnum::VALUE_2},
        {"132", TestEnum::VALUE_3},
        {"133", TestEnum::VALUE_4},
        {"134", TestEnum::VALUE_5},
    };

    constexpr auto converter = atlink::Utils::EnumStringConverter<TestEnum>{};

    GIVEN("An enum variant") {

        auto item = GENERATE(from_range(map));
        auto refStr = item.first;
        auto variant = item.second;

        WHEN("Converted to string") {

            std::array<char, 32U> buf{};
            auto nbytes = converter.stringify(variant, buf);
            auto actual = std::string_view{buf.data(), nbytes};

            THEN("The appropriate string is returned") {
                REQUIRE(refStr == actual);
            }

            THEN("The number of bytes returned is correct") {
                REQUIRE(refStr.size() == nbytes);
            }
        }
    }

    GIVEN("A valid string") {

        auto item = GENERATE(from_range(map));
        auto refStr = item.first;
        auto refVariant = item.second;

        WHEN("Converted to enum") {

            auto variant = TestEnum{};
            auto nbytes = converter.parse(variant, refStr);

            THEN("The associated variant is returned") {
                REQUIRE(refVariant == variant);
            }

            THEN("The number of bytes returned is correct") {
                REQUIRE(refStr.size() == nbytes);
            }
        }
    }

    GIVEN("Invalid strings for parsing") {
        auto str = GENERATE(std::string_view{"abc"},
                            std::string_view{"12x34"},
                            std::string_view{"999"},
                            std::string_view{""});

        WHEN("Converted to enum") {
            const auto original = TestEnum::VALUE_3;
            auto variant = original;
            auto nbytes = converter.parse(variant, str);

            THEN("Returns 0 bytes") {
                REQUIRE(0U == nbytes);
            }

            THEN("Doesn't have side-effects") {
                REQUIRE(original == variant);
            }
        }
    }

    GIVEN("Buffer exactly fitting the output but no space for termination") {
        WHEN("Converting VALUE_1 (130) to 3-char buffer") {
            std::array<char, 3U> buf{};
            auto nbytes = converter.stringify(TestEnum::VALUE_1, buf);

            THEN("Conversion fails") {
                REQUIRE(nbytes == 0U);
            }
        }
    }

    GIVEN("Zero-sized buffer") {
        WHEN("Converting any enum") {
            std::array<char, 0> buf{};
            auto nbytes = converter.stringify(TestEnum::VALUE_1, buf);

            THEN("Returns 0") {
                REQUIRE(nbytes == 0);
            }
        }
    }
}
