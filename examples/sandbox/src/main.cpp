
#include <iostream>
#include <optional>
#include <string>
#include <variant>

#include <atlink/core/Command.h>
#include <atlink/core/Response.h>
#include <atlink/core/Types.h>

#include <atlink/core/Enum.h>

#include <atlink/protocols/standard/Cpin.h>

class Deserializer : public atlink::AInputVisitor {
    const std::string input;

    size_t length = 0;
    bool valid = true;

  public:
    Deserializer(const std::string &input) : input(input) {}
    ~Deserializer() {
        std::cout << "deserialized " << length << " bytes" << std::endl;
    }

    void visit(const atlink::Tag &tag) {
        auto ssstr = tag.asStr();
        valid = (0U == input.find(tag.asStr()));
        if (valid) {
            length += strlen(tag.asStr());
        }
    }

    void visit(const atlink::Term &term) {
        valid = valid && (input.at(length) == '\r');
        valid = valid && (input.at(length + 1) == '\n');
        if (valid) {
            length += 2;
        }
    }

    void visit(const char *str) {
        valid = valid && (input.substr(length, strlen(str)) == str);
        if (valid) {
            length += strlen(str);
        }
    }

    void visit(atlink::AEnum &e) {
        valid = valid && e.fromStr(input.substr(length).c_str());
        if (valid) {
            length += strlen(e.asStr());
        }
    }

    void visit(int i) {}
};

class Ok : public ATL_NS::AResponse {
  public:
    Ok() : ATL_NS::AResponse("OK") {}
    ~Ok() = default;
    void accept(ATL_NS::AInputVisitor &visitor) override {
        APacket::accept(visitor);
    }
};

class CmeError : public atlink::AResponse {
  public:
    enum class Code {
        CME_ERR_0,
        CME_ERR_1,
        CME_ERR_2,
    };

    ATL_NS::Enum<Code> code{};

    CmeError() : ATL_NS::AResponse("+CME ERROR: ") {}
    ~CmeError() = default;
    void accept(ATL_NS::AInputVisitor &visitor) override {
        APacket::accept(visitor, code);
    }
};

template <>
struct atlink::EnumTraits<CmeError::Code> {
    static constexpr const char *ERR0_STR = "err0";
    static constexpr size_t ERR0_STR_LEN = 4;
    static constexpr const char *ERR1_STR = "err1";
    static constexpr size_t ERR1_STR_LEN = 4;
    static constexpr const char *ERR2_STR = "err2";
    static constexpr size_t ERR2_STR_LEN = 4;

    static const char *toString(CmeError::Code value) {
        switch (value) { // ← Compiler generates jump table or conditional branches
        case CmeError::Code::CME_ERR_0:
            return ERR0_STR;
        case CmeError::Code::CME_ERR_1:
            return ERR1_STR;
        case CmeError::Code::CME_ERR_2:
            return ERR2_STR;
        default:
            return "";
        }
    }

    static std::optional<CmeError::Code> fromString(const char *str) {
        if (0 == std::strncmp(str, ERR0_STR, ERR0_STR_LEN)) {
            return CmeError::Code::CME_ERR_0;
        } else if (0 == std::strncmp(str, ERR1_STR, ERR1_STR_LEN)) {
            return CmeError::Code::CME_ERR_1;
        } else if (0 == std::strncmp(str, ERR2_STR, ERR2_STR_LEN)) {
            return CmeError::Code::CME_ERR_2;
        } else {
            return std::nullopt;
        }
    }
};

using Result = std::variant<Ok, CmeError>;

Result exchange(const ATL_NS::ACommand &cmd, ATL_NS::AResponse &res) {
    Deserializer deserializer("+CME ERROR: err1\r\n");
    CmeError cme{};
    cme.accept(deserializer);
    std::cout << "In exchange - code: " << cme.code.asStr() << std::endl;
    return cme;
}

template <typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
Overload(Ts...) -> Overload<Ts...>;

int main(int argc, char *argv[]) {

    auto resolver = Overload{[](const Ok &) {
                                 std::cout << "Ok" << std::endl;
                             },
                             [](const CmeError &e) {
                                 std::cout << "CME error::: " << e.code.asStr() << std::endl;
                             }};
    auto cmd = atlink::Proto::Std::CpinRead{};
    auto res = atlink::Proto::Std::CpinReadResponse{};
    auto result = exchange(cmd, res);

    std::visit(resolver, result);

    return 0;
}
