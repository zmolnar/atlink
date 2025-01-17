

#include <atlink/core/Urc.h>
#include <iostream>
#include <protocols/standard/Cpin.h>
#include <sstream>

namespace {

atl::UrcParser<atl::Proto::Std::CpinReadResponse> parser{};

class Serializer : public atl::AOutputVisitor {

  std::ostringstream output{};

  template <typename T> void visitImpl(T &&value) { output << value << ","; }

public:

  const std::string get() const { return output.str(); }

  void visit(const atl::Tag &tag) override { output << tag.asStr(); }

  void visit(const atl::Term &term) override {
    auto str = output.str();
    str.pop_back();
    output.clear();
    output.str("");
    output << str << "\r\n";
  }

  void visit(const char *str) override { visitImpl(str); }

  void visit(int i) override { visitImpl(i); }

  void visit(const atl::AEnum &e) override { visitImpl(e.asStr()); }

  ~Serializer() = default;
};

class Deserializer : public atl::AInputVisitor {
  std::ostringstream output{};

  template <typename T> void visitImpl(T &&value) { output << value << ","; }

public:

  const std::string get() const { return output.str(); }

  bool visit(const atl::Tag &tag) override {
    output << tag.asStr();
    return false;
  }

  bool visit(const atl::Term &term) override {
    auto str = output.str();
    str.pop_back();
    output.clear();
    output.str("");
    output << str << "\r\n";
    return true;
  }

  bool visit(const char *str) override {
    visitImpl(str);
    return true;
  }

  bool visit(int i) override {
    visitImpl(i);
    return true;
  }

  bool visit(atl::AEnum &e) override {
    output << "," << e.asInt() << ":" << e.asStr();
    return true;
  }

  ~Deserializer() = default;
};

} // namespace

int main() {
  // {
  //   auto ser = Serializer{};
  //   auto cpinRead = atl::Proto::Std::CpinRead{};
  //   cpinRead.accept(ser);
  //   std::cout << ser.get() << std::endl;
  // }

  // {
  //   auto ser = Serializer{};
  //   auto cpinWrite = atl::Proto::Std::CpinWrite{};
  //   cpinWrite.accept(ser);
  //   std::cout << ser.get() << std::endl;
  // }

  {
    auto deser = Deserializer{};
    auto cpinReadResponse = atl::Proto::Std::CpinReadResponse{};
    cpinReadResponse.accept(deser);
    std::cout << deser.get() << std::endl;
  }
}