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

namespace {

class TestResponse : public ATL_NS::Core::MultiLineResponse {
  public:
    class Line : public ATL_NS::Core::Line {
      public:
        ATL_NS::Core::LineString<32U> lineString;

        bool accept(ATL_NS::Core::AResponseVisitor &visitor) override {
            return ATL_NS::Core::Line::acceptImpl(visitor, lineString);
        }
    };

    Line line1{};
    Line line2{};
    Line line3{};

    TestResponse() : ATL_NS::Core::MultiLineResponse("+TEST:") {}

    bool accept(ATL_NS::Core::AResponseVisitor &visitor) override {
        return ATL_NS::Core::MultiLineResponse::acceptImpl(visitor, line1, line2, line3);
    }
};

} // namespace

SCENARIO("Multi-line response is processed") {
    GIVEN("A multi-line response") {
        auto res = TestResponse{};

        WHEN("Response deserialized") {
            std::string input{"+TEST:\r\nline one\r\nline two\r\nline three\r\n\r\nOK\r\n"};
            auto deserializer = ATL_NS::Utils::Deserializer{input};
            auto success = res.accept(deserializer);
            THEN("All lines are fetched properly") {
                REQUIRE(success);
                REQUIRE(std::string_view{"line one"} == res.line1.lineString.view());
                REQUIRE(std::string_view{"line two"} == res.line2.lineString.view());
                REQUIRE(std::string_view{"line three"} == res.line3.lineString.view());
            }
        }

        WHEN("Response has a leading CRLF before the header") {
            std::string input{"\r\n+TEST:\r\nline one\r\nline two\r\nline three\r\n\r\nOK\r\n"};
            auto deserializer = ATL_NS::Utils::Deserializer{input};
            auto success = res.accept(deserializer);
            THEN("The optional leading CRLF is tolerated and lines are still parsed") {
                REQUIRE(success);
                REQUIRE(std::string_view{"line one"} == res.line1.lineString.view());
                REQUIRE(std::string_view{"line two"} == res.line2.lineString.view());
                REQUIRE(std::string_view{"line three"} == res.line3.lineString.view());
            }
        }

        WHEN("Response has an invalid header tag") {
            std::string input{"+TESX:\r\nline one\r\nline two\r\nline three\r\n\r\nOK\r\n"};
            auto deserializer = ATL_NS::Utils::Deserializer{input};
            auto success = res.accept(deserializer);
            THEN("The response is rejected") {
                REQUIRE_FALSE(success);
                // We do not assume anything about partial consumption or line contents
                // on failure, since parsing is not transactional.
            }
        }

        WHEN("Response contains fewer lines than expected") {
            std::string input{"+TEST:\r\nonly one\r\nonly two\r\n"}; // missing third line
            auto deserializer = ATL_NS::Utils::Deserializer{input};
            auto success = res.accept(deserializer);
            THEN("The response is rejected because not all lines can be parsed") {
                REQUIRE_FALSE(success);
                // Again, we avoid assumptions about partially filled lines.
            }
        }
    }
}