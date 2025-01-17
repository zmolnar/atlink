
#include <core/ATLink.h>
#include <core/Urc.h>
#include <iostream>
#include <protocols/standard/Cpin.h>
#include <sstream>

namespace {

ATL::UrcParser<ATL::Proto::Std::CpinReadResponse> parser{};

class Serializer : public ATL::AOutputVisitor {

  std::ostringstream output{};

  template <typename T> void visitImpl(T &&value) { output << value << ","; }

public:

  const std::string get() const { return output.str(); }

  void visit(const ATL::Tag &tag) override { output << tag.asStr(); }

  void visit(const ATL::Term &term) override {
    auto str = output.str();
    str.pop_back();
    output.clear();
    output.str("");
    output << str << "\r\n";
  }

  void visit(const char *str) override { visitImpl(str); }

  void visit(int i) override { visitImpl(i); }

  void visit(const ATL::AEnum &e) override { visitImpl(e.asStr()); }

  ~Serializer() = default;
};

class Deserializer : public ATL::AInputVisitor {
public:
  void visit(const ATL::Tag &tag) override { std::cout << tag.asStr(); }

  void visit(const ATL::Term &term) override { std::cout << std::endl; }

  void visit(const char *str) override {}

  void visit(int i) override {}

  void visit(ATL::AEnum &e) override {}

  ~Deserializer() = default;
};

} // namespace

int main() {
  // {
  //   auto ser = Serializer{};
  //   auto cpinRead = ATL::Proto::Std::CpinRead{};
  //   cpinRead.accept(ser);
  //   std::cout << ser.get() << std::endl;
  // }

  // {
  //   auto ser = Serializer{};
  //   auto cpinWrite = ATL::Proto::Std::CpinWrite{};
  //   cpinWrite.accept(ser);
  //   std::cout << ser.get() << std::endl;
  // }

  {
    auto deser = Deserializer{};
    auto cpinReadResponse = ATL::Proto::Std::CpinReadResponse{};
    cpinReadResponse.accept(deser);
    parser.accept(deser);
  }
}