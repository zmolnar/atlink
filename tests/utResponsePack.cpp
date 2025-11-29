// tests/utResponsePack.cpp
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

#include "atlink/core/ResponsePack.h"
#include "atlink/utils/Deserializer.h"

#include <catch2/catch_all.hpp>
#include <cstring>

namespace {

using ATL_NS::Core::APacket;
using ATL_NS::Core::AResponseVisitor;
using ATL_NS::Core::QuotedField;
using ATL_NS::Core::Response;

class FooResponse : public Response {
  public:
    int num{0};
    QuotedField<32U> str{};

    FooResponse() : Response("+FOO:") {}

    bool accept(AResponseVisitor &v) override {
        return Response::acceptImpl(v, num, str.storage());
    }
};

class BarResponse : public Response {
  public:
    int value{0};

    BarResponse() : Response("+BAR:") {}

    bool accept(AResponseVisitor &v) override {
        return Response::acceptImpl(v, value);
    }
};

class BazResponse : public Response {
  public:
    BazResponse() : Response("+BAZ:") {}

    bool accept(AResponseVisitor &v) override {
        return Response::acceptImpl(v);
    }
};

class DupIntOnly : public Response {
  public:
    int n{0};
    DupIntOnly() : Response("+DUP:") {}
    bool accept(AResponseVisitor &v) override {
        return Response::acceptImpl(v, n);
    }
};

class DupIntStr : public Response {
  public:
    int n{0};
    QuotedField<32U> s{};
    DupIntStr() : Response("+DUP:") {}
    bool accept(AResponseVisitor &v) override {
        return Response::acceptImpl(v, n, s.storage());
    }
};

} // namespace

SCENARIO("ResponsePack parses the first matching response type") {

    GIVEN("A pack with Foo, Bar, Baz in that order") {
        ATL_NS::Core::ResponsePack<FooResponse, BarResponse, BazResponse> pack{};

        WHEN("Input matches the first (FooResponse)") {
            atlink::Utils::Deserializer d{"+FOO: 7, \"hello\"\r\n"};
            const bool ok = pack.accept(d);

            THEN("FooResponse is held with correct data") {
                REQUIRE(ok);
                REQUIRE(pack.holds<FooResponse>());
                auto *foo = pack.getIf<FooResponse>();
                REQUIRE(foo != nullptr);
                REQUIRE(foo->num == 7);
                REQUIRE(std::string_view{"hello"} == foo->str.view());
                // +FOO: 7, "hello"\r\n  -> sanity check consumed bytes
                REQUIRE(d.consumed() > 0);
            }
        }

        WHEN("Input matches the middle (BarResponse)") {
            atlink::Utils::Deserializer d{"+BAR:   42  \r\n"};
            const bool ok = pack.accept(d);

            THEN("BarResponse is held with correct data") {
                REQUIRE(ok);
                REQUIRE(pack.holds<BarResponse>());
                auto *bar = pack.getIf<BarResponse>();
                REQUIRE(bar != nullptr);
                REQUIRE(bar->value == 42);
                REQUIRE(d.consumed() > 0);
            }
        }

        WHEN("Input matches the last (BazResponse)") {
            atlink::Utils::Deserializer d{"+BAZ:\r\n"};
            const bool ok = pack.accept(d);

            THEN("BazResponse is held") {
                REQUIRE(ok);
                REQUIRE(pack.holds<BazResponse>());
                auto *baz = pack.getIf<BazResponse>();
                REQUIRE(baz != nullptr);
                REQUIRE(d.consumed() > 0);
            }
        }

        WHEN("Input matches none of the alternatives") {
            atlink::Utils::Deserializer d{"+QUX: 1\r\n"};
            const bool ok = pack.accept(d);

            THEN("Parsing fails and value remains monostate") {
                REQUIRE_FALSE(ok);
                REQUIRE_FALSE(pack.holds<FooResponse>());
                REQUIRE_FALSE(pack.holds<BarResponse>());
                REQUIRE_FALSE(pack.holds<BazResponse>());
            }
        }

        WHEN("Reset is called after a successful parse") {
            atlink::Utils::Deserializer d1{"+BAR: 99\r\n"};
            REQUIRE(pack.accept(d1));
            REQUIRE(pack.holds<BarResponse>());

            pack.reset();

            THEN("The pack holds monostate again and can be reused") {
                REQUIRE_FALSE(pack.holds<BarResponse>());

                atlink::Utils::Deserializer d2{"+FOO: 1, \"x\"\r\n"};
                REQUIRE(pack.accept(d2));
                REQUIRE(pack.holds<FooResponse>());
                auto *foo = pack.getIf<FooResponse>();
                REQUIRE(foo != nullptr);
                REQUIRE(foo->num == 1);
                REQUIRE(std::string_view{"x"} == foo->str.view());
            }
        }
    }
}

SCENARIO("ResponsePack respects ordering with duplicate tags") {

    GIVEN("A pack with two responses sharing the same tag +DUP:") {
        // Note order: first tries int-only, then int+str
        ATL_NS::Core::ResponsePack<DupIntOnly, DupIntStr> pack{};

        WHEN("Input has only an int") {
            atlink::Utils::Deserializer d{"+DUP: 123\r\n"};
            const bool ok = pack.accept(d);

            THEN("The first (int-only) wins") {
                REQUIRE(ok);
                REQUIRE(pack.holds<DupIntOnly>());
                auto *r = pack.getIf<DupIntOnly>();
                REQUIRE(r != nullptr);
                REQUIRE(r->n == 123);
            }
        }

        WHEN("Input has int and string") {
            atlink::Utils::Deserializer d{"+DUP: 5, \"five\"\r\n"};
            const bool ok = pack.accept(d);

            THEN("The first fails and the second (int+str) succeeds") {
                REQUIRE(ok);
                REQUIRE(pack.holds<DupIntStr>());
                auto *r = pack.getIf<DupIntStr>();
                REQUIRE(r != nullptr);
                REQUIRE(r->n == 5);
                REQUIRE(std::string_view{"five"} == r->s.view());
            }
        }
    }
}

SCENARIO("ResponsePack with a single response type works and accessors behave correctly") {

    GIVEN("A pack with a single BarResponse") {
        ATL_NS::Core::ResponsePack<BarResponse> pack{};

        THEN("Initially it holds monostate") {
            const auto &v = pack.getValue();
            // We can't refer to monostate via ResponsePack::Variant directly here
            // but we can infer it's not any of our response types:
            REQUIRE_FALSE(pack.holds<BarResponse>());
            REQUIRE(pack.getIf<BarResponse>() == nullptr);
        }

        WHEN("A matching BarResponse input is parsed") {
            atlink::Utils::Deserializer d{"+BAR: 17\r\n"};
            const bool ok = pack.accept(d);

            THEN("The single response is held and accessors reflect that") {
                REQUIRE(ok);
                REQUIRE(pack.holds<BarResponse>());

                // non-const getIf
                auto *bar = pack.getIf<BarResponse>();
                REQUIRE(bar != nullptr);
                REQUIRE(bar->value == 17);

                // const getIf
                const auto &cpack = pack;
                const auto *cbar = cpack.getIf<BarResponse>();
                REQUIRE(cbar != nullptr);
                REQUIRE(cbar->value == 17);

                // getValue() should now hold BarResponse
                const auto &var = pack.getValue();
                REQUIRE(std::holds_alternative<BarResponse>(var));
            }
        }

        WHEN("Input does not match BarResponse") {
            atlink::Utils::Deserializer d{"+FOO: 1, \"x\"\r\n"};
            const bool ok = pack.accept(d);

            THEN("Parsing fails and the pack remains in monostate") {
                REQUIRE_FALSE(ok);
                REQUIRE_FALSE(pack.holds<BarResponse>());
                REQUIRE(pack.getIf<BarResponse>() == nullptr);
            }
        }

        WHEN("Reset is called after a successful parse") {
            atlink::Utils::Deserializer d1{"+BAR: 99\r\n"};
            REQUIRE(pack.accept(d1));
            REQUIRE(pack.holds<BarResponse>());

            pack.reset();

            THEN("The pack no longer holds BarResponse and behaves as empty") {
                REQUIRE_FALSE(pack.holds<BarResponse>());
                REQUIRE(pack.getIf<BarResponse>() == nullptr);

                // And it can be reused
                atlink::Utils::Deserializer d2{"+BAR: 5\r\n"};
                REQUIRE(pack.accept(d2));
                REQUIRE(pack.holds<BarResponse>());
                auto *bar = pack.getIf<BarResponse>();
                REQUIRE(bar != nullptr);
                REQUIRE(bar->value == 5);
            }
        }
    }
}