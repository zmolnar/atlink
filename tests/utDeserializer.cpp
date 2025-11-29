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
#include <cstring>

SCENARIO("Sequence can be deserialized") {

    using atlink::Core::Sequence;
    const Sequence seq{"ABC"};

    GIVEN("A valid sequence at the beginning of the input") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"ABC"};
            auto success = deserializer.visit(seq);
            THEN("The sequence is recognized and the correct number of bytes is reported") {
                REQUIRE(success);
                REQUIRE(3U == deserializer.consumed());
            }
        }
    }

    GIVEN("An invalid sequence") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"ABX"};
            auto success = deserializer.visit(seq);
            THEN("The deserializer reports invalidity and consumes nothing") {
                REQUIRE_FALSE(success);
                REQUIRE(0U == deserializer.consumed());
            }
        }
    }

    GIVEN("A valid sequence with leading whitespaces") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"  \tABC"};
            auto success = deserializer.visit(seq);
            THEN("Leading whitespace is skipped and the correct number of bytes is reported") {
                REQUIRE(success);
                // 3 characters of whitespace + 3 for "ABC"
                REQUIRE(6U == deserializer.consumed());
            }
        }
    }

    GIVEN("Input containing only whitespaces") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"   \t   "};
            auto success = deserializer.visit(seq);
            THEN("No sequence is matched and all input is considered consumed after skipping") {
                REQUIRE_FALSE(success);
                REQUIRE(7U == deserializer.consumed());
            }
        }
    }
}

SCENARIO("Integer can be deserialized") {

    GIVEN("A valid decimal integer without leading whitespace") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"42"};
            int value = 0;
            auto success = deserializer.visit(value);
            THEN("The integer is parsed and the correct number of characters is consumed") {
                REQUIRE(success);
                REQUIRE(value == 42);
                REQUIRE(2U == deserializer.consumed());
            }
        }
    }

    GIVEN("A valid decimal integer with leading whitespace") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"   1234"};
            int value = 0;
            auto success = deserializer.visit(value);
            THEN("Leading whitespace is skipped and the integer is parsed") {
                REQUIRE(success);
                REQUIRE(value == 1234);
                // 3 spaces + 4 digits
                REQUIRE(7U == deserializer.consumed());
            }
        }
    }

    GIVEN("An invalid integer") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"abc123"};
            int value = 0;
            auto success = deserializer.visit(value);
            THEN("The deserializer reports invalidity and does not consume input") {
                REQUIRE_FALSE(success);
                REQUIRE(value == 0); // unchanged
                REQUIRE(0U == deserializer.consumed());
            }
        }
    }

    GIVEN("An empty input") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{""};
            int value = 0;
            auto success = deserializer.visit(value);
            THEN("Deserialization fails and nothing is consumed") {
                REQUIRE_FALSE(success);
                REQUIRE(value == 0);
                REQUIRE(0U == deserializer.consumed());
            }
        }
    }

    GIVEN("Input that contains only whitespace") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"   \t"};
            int value = 0;
            auto success = deserializer.visit(value);
            THEN("No integer is parsed and all input is consumed as whitespace") {
                REQUIRE_FALSE(success);
                REQUIRE(value == 0);
                REQUIRE(4U == deserializer.consumed());
            }
        }
    }
}

SCENARIO("Quoted string can be deserialized") {

    using atlink::Core::QuotedField;

    GIVEN("A valid quoted string") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"\"HELLO\""};
            QuotedField<16> field;
            auto storage = field.storage();
            auto success = deserializer.visit(storage);
            THEN("The string literal is parsed without quotes and consumed size includes quotes") {
                REQUIRE(success);
                REQUIRE(field.view() == "HELLO");
                // 2 quotes + 5 letters
                REQUIRE(7U == deserializer.consumed());
            }
        }
    }

    GIVEN("A valid quoted string with leading whitespace") {
        WHEN("Deserialized") {
            atlink::Utils::Deserializer deserializer{"  \"ABC\""};
            QuotedField<16> field;
            auto storage = field.storage();
            auto success = deserializer.visit(storage);
            THEN("Leading whitespace is skipped and the string is parsed correctly") {
                REQUIRE(success);
                REQUIRE(field.view() == "ABC");
                // 2 spaces + 2 quotes + 3 letters
                REQUIRE(7U == deserializer.consumed());
            }
        }
    }

    GIVEN("A quoted string longer than the available storage") {
        WHEN("Deserialized") {
            // 8 characters between the quotes, but storage is smaller
            atlink::Utils::Deserializer deserializer{"\"TOO_LONG\""};
            QuotedField<4> field; // very small buffer
            auto storage = field.storage();
            auto success = deserializer.visit(storage);
            THEN("The buffer remains effectively empty (no useful content copied)") {
                // Current implementation reports success if it finds two quotes,
                // but does not copy data when it does not fit.
                REQUIRE(success);
                REQUIRE(field.view().empty());
                // parseStringLiteral returns length + 2, but length is reset to 0
                // when the string is too long, so only the quotes are counted.
                REQUIRE(2U == deserializer.consumed());
            }
        }
    }
}

SCENARIO("LineText can be deserialized") {

    using atlink::Core::LineText;
    using atlink::Core::MutableBuffer;

    GIVEN("Input without CRLF and sufficient buffer size") {
        const char *input = "Hello, world!";
        WHEN("Deserialized into LineText") {
            atlink::Utils::Deserializer deserializer{input};

            char buf[32] = {};
            LineText line{MutableBuffer{buf, sizeof(buf)}};

            auto success = deserializer.visit(line);

            THEN("All characters are copied until the end of input") {
                REQUIRE(success);
                REQUIRE(deserializer.consumed() == std::strlen(input));

                std::string_view lineView{buf, deserializer.consumed()};
                REQUIRE(lineView == input);
            }
        }
    }

    GIVEN("Input containing CRLF before the end") {
        const char *input = "ABC\r\nDEF";
        WHEN("Deserialized into LineText") {
            atlink::Utils::Deserializer deserializer{input};

            char buf[16] = {};
            LineText line{MutableBuffer{buf, sizeof(buf)}};

            auto success = deserializer.visit(line);

            THEN("Characters are copied only up to CRLF") {
                REQUIRE(success);
                // Only "ABC" is taken before CRLF
                REQUIRE(3U == deserializer.consumed());

                std::string_view lineView{buf, 3U};
                REQUIRE(lineView == "ABC");
            }
        }
    }

    GIVEN("Input longer than the LineText buffer") {
        // TODO:
        const char *input = "1234567890";
        WHEN("Deserialized into a smaller LineText buffer") {
            atlink::Utils::Deserializer deserializer{input};

            char buf[5] = {}; // smaller than input
            LineText line{MutableBuffer{buf, sizeof(buf)}};

            auto success = deserializer.visit(line);

            THEN("Only as many characters as fit into the buffer are copied") {
                REQUIRE(success);
                REQUIRE(deserializer.consumed() == sizeof(buf));

                std::string_view lineView{buf, deserializer.consumed()};
                REQUIRE(lineView == std::string_view(input, sizeof(buf)));
            }
        }
    }
}

SCENARIO("Enum can be deserialized") {

    using atlink::Core::Enum;

    // A simple numeric enum; no MapProvider specialization, so numeric converter is used.
    enum class TestEnum : int { Zero = 0, One = 1, Two = 2 };

    GIVEN("A valid numeric representation for a numeric enum") {
        WHEN("Deserialized via AEnum interface") {
            atlink::Utils::Deserializer deserializer{"1"};
            Enum<TestEnum> e;
            auto &asAEnum = static_cast<atlink::Core::AEnum &>(e);

            auto success = deserializer.visit(asAEnum);

            THEN("The enum value is parsed and some input is consumed") {
                REQUIRE(success);
                REQUIRE(deserializer.consumed() > 0U);

                // We expect the underlying numeric value to be 1
                auto value = static_cast<TestEnum>(e);
                REQUIRE(value == TestEnum::One);
            }
        }
    }

    GIVEN("An invalid numeric representation for the enum") {
        WHEN("Deserialized via AEnum interface") {
            atlink::Utils::Deserializer deserializer{"abc"};
            Enum<TestEnum> e;
            e = TestEnum::Two; // set to a non-default value

            auto &asAEnum = static_cast<atlink::Core::AEnum &>(e);
            auto success = deserializer.visit(asAEnum);

            THEN("The deserializer reports invalidity and does not consume input") {
                REQUIRE_FALSE(success);
                REQUIRE(0U == deserializer.consumed());
                // value should remain unchanged
                auto value = static_cast<TestEnum>(e);
                REQUIRE(value == TestEnum::Two);
            }
        }
    }
}

SCENARIO("Deserializer can be rewound") {

    using atlink::Core::Sequence;
    const Sequence seq{"ABC"};

    GIVEN("A deserializer that has already consumed some input") {
        atlink::Utils::Deserializer deserializer{"ABC 123"};

        // Consume the sequence first
        REQUIRE(deserializer.visit(seq));
        REQUIRE(3U == deserializer.consumed());

        // Consume the integer to advance further
        int value = 0;
        REQUIRE(deserializer.visit(value));
        REQUIRE(value == 123);
        REQUIRE(deserializer.consumed() > 3U);

        WHEN("Rewind is called") {
            deserializer.rewind();

            THEN("The consumed length is reset to zero") {
                REQUIRE(0U == deserializer.consumed());
            }
        }
    }
}