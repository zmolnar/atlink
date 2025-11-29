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

#include "atlink/core/Urc.h"
#include "atlink/utils/Deserializer.h"

#include <catch2/catch_all.hpp>

namespace {

using namespace ATL_NS::Core;
using namespace ATL_NS::Utils;

class FooUrc : public Response {
  public:
    int value{0};

    FooUrc() : Response("+FOO:") {}

    bool accept(AResponseVisitor &visitor) override {
        // "+FOO:" <COMMA optional> <int> <TERM>
        return Response::acceptImpl(visitor, value);
    }
};

template <typename UrcPack>
class TestUrcDispatcher : public AUrcDispatcher {
  public:
    UrcPack pack;

    std::size_t dispatch(ReadOnlyText str) override {
        ATL_NS::Utils::Deserializer deserializer{str};
        const bool ok = pack.accept(deserializer);
        return (ok ? deserializer.consumed() : 0U);
    }
};

} // namespace

SCENARIO("Known URC is parsed into the Urc<> pack") {

    GIVEN("A dispatcher with FooUrc as a known URC type") {
        using UrcPack = Urc<FooUrc>;
        TestUrcDispatcher<UrcPack> dispatcher{};

        WHEN("A +FOO URC line is dispatched") {
            constexpr const char *line = "+FOO: 123\r\n";
            const ATL_NS::Core::ReadOnlyText input{line, std::char_traits<char>::length(line)};

            auto consumed = dispatcher.dispatch(input);

            THEN("The full line is consumed and FooUrc is stored") {
                REQUIRE(consumed == input.size());
                REQUIRE(dispatcher.pack.template holds<FooUrc>());

                const FooUrc *urc = dispatcher.pack.template getIf<FooUrc>();
                REQUIRE(urc != nullptr);
                REQUIRE(urc->value == 123);
            }
        }
    }
}

SCENARIO("Unknown URC is captured by AnyUrc as fallback") {

    GIVEN("A dispatcher with FooUrc and AnyUrc as fallback") {
        using UrcPack = Urc<FooUrc>;
        TestUrcDispatcher<UrcPack> dispatcher{};

        WHEN("An unknown URC line is dispatched") {
            constexpr const char *line = "+BAR: some payload\r\n";
            const ATL_NS::Core::ReadOnlyText input{line, std::char_traits<char>::length(line)};

            auto consumed = dispatcher.dispatch(input);

            THEN("The full line is consumed by AnyUrc") {
                REQUIRE(consumed == input.size());
                REQUIRE(dispatcher.pack.template holds<AnyUrc>());

                const AnyUrc *urc = dispatcher.pack.template getIf<AnyUrc>();
                REQUIRE(urc != nullptr);
                const char *payloadCStr = urc->storage.data();
                REQUIRE(std::string_view{payloadCStr}.find("+BAR: some payload") !=
                        std::string_view::npos);
            }
        }
    }
}

SCENARIO("Malformed known URC falls back to AnyUrc") {

    GIVEN("A dispatcher with FooUrc and AnyUrc as fallback") {
        using UrcPack = Urc<FooUrc>;
        TestUrcDispatcher<UrcPack> dispatcher{};

        WHEN("A +FOO URC has a non-integer payload") {
            constexpr const char *line = "+FOO: not_an_int\r\n";
            const ATL_NS::Core::ReadOnlyText input{line, std::char_traits<char>::length(line)};

            auto consumed = dispatcher.dispatch(input);

            THEN("FooUrc parsing fails and AnyUrc captures the line") {
                REQUIRE(consumed == input.size());
                REQUIRE(dispatcher.pack.template holds<AnyUrc>());

                const FooUrc *foo = dispatcher.pack.template getIf<FooUrc>();
                REQUIRE(foo == nullptr);

                const AnyUrc *urc = dispatcher.pack.template getIf<AnyUrc>();
                REQUIRE(urc != nullptr);
                const char *payloadCStr = urc->storage.data();
                REQUIRE(std::string_view{payloadCStr}.find("+FOO: not_an_int") !=
                        std::string_view::npos);
            }
        }
    }
}

SCENARIO("URC without terminator is rejected by all handlers") {

    GIVEN("A dispatcher with FooUrc and AnyUrc as fallback") {
        using UrcPack = Urc<FooUrc>;
        TestUrcDispatcher<UrcPack> dispatcher{};

        WHEN("A +FOO URC line is missing CRLF") {
            constexpr const char *line = "+FOO: 123"; // no \r\n terminator
            const ATL_NS::Core::ReadOnlyText input{line, std::char_traits<char>::length(line)};

            auto consumed = dispatcher.dispatch(input);

            THEN("Parsing fails and no URC is stored") {
                REQUIRE(consumed == 0U);
                REQUIRE_FALSE(dispatcher.pack.template holds<FooUrc>());
                REQUIRE_FALSE(dispatcher.pack.template holds<AnyUrc>());
            }
        }
    }
}

SCENARIO("Dispatcher can be reused for multiple URCs") {

    GIVEN("A dispatcher with FooUrc and AnyUrc as fallback") {
        using UrcPack = Urc<FooUrc>;
        TestUrcDispatcher<UrcPack> dispatcher{};

        WHEN("A known URC is dispatched first") {
            constexpr const char *line1 = "+FOO: 10\r\n";
            const ATL_NS::Core::ReadOnlyText input1{line1, std::char_traits<char>::length(line1)};
            auto consumed1 = dispatcher.dispatch(input1);

            REQUIRE(consumed1 == input1.size());
            REQUIRE(dispatcher.pack.template holds<FooUrc>());
            const FooUrc *foo1 = dispatcher.pack.template getIf<FooUrc>();
            REQUIRE(foo1 != nullptr);
            REQUIRE(foo1->value == 10);

            AND_WHEN("An unknown URC is dispatched afterwards") {
                constexpr const char *line2 = "+XYZ: something\r\n";
                const ATL_NS::Core::ReadOnlyText input2{line2,
                                                        std::char_traits<char>::length(line2)};
                auto consumed2 = dispatcher.dispatch(input2);

                THEN("The new URC overwrites the previous one in the pack") {
                    REQUIRE(consumed2 == input2.size());
                    REQUIRE_FALSE(dispatcher.pack.template holds<FooUrc>());
                    REQUIRE(dispatcher.pack.template holds<AnyUrc>());
                    const AnyUrc *any = dispatcher.pack.template getIf<AnyUrc>();
                    REQUIRE(any != nullptr);
                    const char *payloadCStr = any->storage.data();
                    REQUIRE(std::string_view{payloadCStr}.find("+XYZ: something") !=
                            std::string_view::npos);
                }
            }
        }
    }
}