
#include <atlink/core/Types.h>

#include <iostream>
#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

SCENARIO("Enum can be represented properly") {

    GIVEN("An enum with custom variants") {

        enum class Variants {
            VARIANT_1,
            VARIANT_2,
            VARIANT_3,
        };

        auto variants = {std::make_pair(Variants::VARIANT_1, "variant 1"),
                         std::make_pair(Variants::VARIANT_2, "variant 2"),
                         std::make_pair(Variants::VARIANT_3, "variant 3")};

        auto atlEnum = ATL_NS::Enum<Variants, 3U>{variants};

        WHEN("Enum is constructed") {
            THEN("It's value is initialized to the first variant") {
                REQUIRE(Variants::VARIANT_1 == atlEnum.get());
            }
        }

        WHEN("It's value is set to a new value") {
            for (const auto &variant : variants) {
                atlEnum.set(variant.first);
                THEN("The value is updated correctly") {
                    REQUIRE(variant.first == atlEnum.get());
                }
                THEN("The matching string representation is retrieved") {
                    REQUIRE(std::string{variant.second} == std::string{atlEnum.asStr()});
                }
            }
        }
    }
}