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

using ATL_NS::Core::AInputVisitor;
using ATL_NS::Core::APacket;
using ATL_NS::Core::AResponse;
using ATL_NS::Core::QuotedString;

class FooResponse : public AResponse {
  public:
    int num{0};
    char str[32]{};
    QuotedString strBuf{str};

    FooResponse() : AResponse("+FOO:") {}

    bool accept(AInputVisitor &v) override {
        return AResponse::acceptImpl(v, num, strBuf);
    }
};

class BarResponse : public AResponse {
  public:
    int value{0};

    BarResponse() : AResponse("+BAR:") {}

    bool accept(AInputVisitor &v) override {
        return AResponse::acceptImpl(v, value);
    }
};

class BazResponse : public AResponse {
  public:
    BazResponse() : AResponse("+BAZ:") {}

    bool accept(AInputVisitor &v) override {
        return AResponse::acceptImpl(v);
    }
};

class DupIntOnly : public AResponse {
  public:
    int n{0};
    DupIntOnly() : AResponse("+DUP:") {}
    bool accept(AInputVisitor &v) override {
        return AResponse::acceptImpl(v, n);
    }
};

class DupIntStr : public AResponse {
  public:
    int n{0};
    char s[32]{};
    QuotedString sbuf{s};
    DupIntStr() : AResponse("+DUP:") {}
    bool accept(AInputVisitor &v) override {
        return AResponse::acceptImpl(v, n, sbuf);
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
                REQUIRE(std::strcmp(foo->str, "hello") == 0);
                // +FOO: 7, "hello"\r\n  -> sanity check consumed bytes
                REQUIRE(d.numberOfBytesConsumed() > 0);
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
                REQUIRE(d.numberOfBytesConsumed() > 0);
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
                REQUIRE(d.numberOfBytesConsumed() > 0);
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
                REQUIRE(std::strcmp(foo->str, "x") == 0);
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
                REQUIRE(std::strcmp(r->s, "five") == 0);
            }
        }
    }
}