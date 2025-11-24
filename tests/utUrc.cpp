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

class FooUrc : public AResponse {
  public:
    int value{0};

    FooUrc() : AResponse("+FOO:") {}

    bool accept(AInputVisitor &visitor) override {
        // "+FOO:" <COMMA optional> <int> <TERM>
        return AResponse::acceptImpl(visitor, value);
    }
};

template <typename UrcPack>
class TestUrcDispatcher : public AUrcDispatcher {
  public:
    UrcPack pack;

    std::size_t dispatch(ReadOnlyText str) override {
        ATL_NS::Utils::Deserializer deserializer{str};
        const bool ok = pack.accept(deserializer);
        return (ok ? deserializer.numberOfBytesConsumed() : 0U);
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
                REQUIRE(std::string_view{payloadCStr}.find("+BAR: some payload") != std::string_view::npos);
            }
        }
    }
}