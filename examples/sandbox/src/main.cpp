
#include <iostream>
#include <optional>
#include <string>
#include <variant>

#include "atlink/core/Command.h"
#include "atlink/core/Enum.h"
#include "atlink/core/Response.h"
#include "atlink/core/Types.h"
#include "atlink/protocols/standard/CmeError.h"
#include "atlink/protocols/standard/Cpin.h"

class Deserializer : public atlink::Core::AInputVisitor {

    std::string input;
    size_t length = 0;
    bool valid = true;

  public:
    Deserializer(const char *input) : input(input) {}
    ~Deserializer() {
        std::cout << "deserialized " << length << " bytes" << std::endl;
    }

    void visit(atlink::Core::Tag &tag) {
        auto n = tag.parse(input.substr(length));
        if (0U < n) {
            length += n;
        } else {
            valid = false;
        }
    }

    void visit(atlink::Core::Term &term) {
        auto n = term.parse(input.substr(length));
        if (0U < n) {
            length += n;
        } else {
            valid = false;
        }
    }

    void visit(atlink::Core::MutableBuffer str) {
        return;
    }

    void visit(atlink::Core::ReadOnlyText str) {
        valid = valid && (input.substr(length, str.size()) == str);
        if (valid) {
            length += str.size();
        }
    }

    void visit(atlink::Core::AEnum &e) {
        auto n = e.parse(input.substr(length));
        if (valid && (0U < n)) {
            length += n;
        } else {
            valid = false;
        }
    }

    void visit(int i) {}
};

class Ok : public ATL_NS::Core::AResponse {
  public:
    Ok() : ATL_NS::Core::AResponse("OK") {}
    ~Ok() = default;
    void accept(ATL_NS::Core::AInputVisitor &visitor) override {
        APacket::accept(visitor);
    }
};

using Result = std::variant<Ok, ATL_NS::Proto::Std::CmeError>;

Result exchange(const ATL_NS::Core::ACommand &cmd, ATL_NS::Core::AResponse &res) {

    Deserializer deserializer("+CME ERROR: 141\r\n");
    ATL_NS::Proto::Std::CmeError cme{};
    cme.accept(deserializer);
    char buf[128] = {0};
    cme.code.stringify(buf);
    std::cout << "In exchange - code: " << buf << std::endl;
    return cme;
}

template <typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
Overload(Ts...) -> Overload<Ts...>;

int main(int argc, char *argv[]) {

    using CmeError = ATL_NS::Proto::Std::CmeError;
    auto resolver = Overload{[](const Ok &) {
                                 std::cout << "Ok" << std::endl;
                             },
                             [](const CmeError &e) {
                                 char buf[128] = {0};
                                 e.code.stringify(buf);
                                 std::cout << "CME error::" << buf << std::endl;
                             }};
    auto cmd = atlink::Proto::Std::CpinRead{};
    auto res = atlink::Proto::Std::CpinReadResponse{};
    auto result = exchange(cmd, res);

    std::visit(resolver, result);

    return 0;
}
