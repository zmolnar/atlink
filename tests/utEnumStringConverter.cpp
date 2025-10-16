#include "atlink/utils/EnumStringConverter.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <iostream>

namespace {

enum class TestEnum {
    VALUE_1,
    VALUE_2,
    VALUE_3,
    VALUE_4,
    VALUE_5,
};

using Record = atlink::Utils::EnumStringRecord<TestEnum>;
constexpr std::array map = {
    Record{"value five", TestEnum::VALUE_5},
    Record{"value four", TestEnum::VALUE_4},
    Record{"value one", TestEnum::VALUE_1},
    Record{"value three", TestEnum::VALUE_3},
    Record{"value two", TestEnum::VALUE_2},
};

static_assert(atlink::Utils::isStrictlySortedByString(map),
              "the map shall be sorted by string");

constexpr auto converter = atlink::Utils::EnumStringConverter{map};

} // namespace

SCENARIO("Enum value can be mapped to a specific string") {

    GIVEN("An enum variant") {

        auto variant = GENERATE(TestEnum::VALUE_1,
                                TestEnum::VALUE_2,
                                TestEnum::VALUE_3,
                                TestEnum::VALUE_4,
                                TestEnum::VALUE_5);

        WHEN("It is converted to string") {

            auto actual = converter.toString(variant);

            THEN("The appropriate string is returned") {

                auto matcher = [variant](const Record &item) {
                    return item.second == variant;
                };

                auto it = std::find_if(map.begin(), map.end(), matcher);
                REQUIRE(it != map.end());
                auto expected = it->first;

                REQUIRE(actual == expected);
            }
        }
    }

    GIVEN("A valid string") {

        auto str = GENERATE(std::string_view{"value one\r\n"},
                            std::string_view{"value two\r\n"},
                            std::string_view{"value three\r\n"},
                            std::string_view{"value four\r\n"},
                            std::string_view{"value five\r\n"});

        WHEN("It is converted to enum variant") {

            auto actual = converter.fromString(str);

            THEN("The appropriate variant is returned") {

                auto starts_with = [](std::string_view text,
                                      std::string_view prefix) {
                    return text.size() >= prefix.size() &&
                           text.compare(0, prefix.size(), prefix) == 0;
                };

                auto matcher = [str, starts_with](const Record &item) {
                    return starts_with(str, item.first);
                };

                auto it = std::find_if(map.begin(), map.end(), matcher);
                REQUIRE(it != map.end());

                auto expected = it->second;
                REQUIRE(actual == expected);
            }
        }
    }

    GIVEN("A string not matching to any variant") {
        auto str = std::string_view{"invalid"};
        WHEN("Converted") {
            auto result = converter.fromString(str);
            THEN("Returns nullopt") {
                REQUIRE(!result.has_value());
            }
        }
    }

    GIVEN("Empty string") {
        WHEN("Converted") {
            auto result = converter.fromString("");
            THEN("Returns nullopt") {
                REQUIRE(!result.has_value());
            }
        }
    }

    GIVEN("String longer than any enum") {
        WHEN("Converted") {
            auto result = converter.fromString(
                "this is a very long string that matches nothing");
            THEN("Returns nullopt") {
                REQUIRE(!result.has_value());
            }
        }
    }
}
