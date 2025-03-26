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

#ifndef RESPONSE_H
#define RESPONSE_H

#include <type_traits>
#include <variant>

#include "Packet.h"

#define REFLECT_RESPONSE_NONE()                                                \
  template <typename... Args>                                                  \
  bool accept(ATL::AInputVisitor &visitor, Args &&... args) {                  \
    bool result = true;                                                        \
    ((result = result && visitor.visit(std::forward<Args>(args))) && ...);     \
    return result;                                                             \
  }                                                                            \
  bool accept(ATL::AInputVisitor &visitor) override {                          \
    return accept(visitor, tag, term);                                         \
  }

// TODO: handle zero arguments and remove the macro above
#define REFLECT_RESPONSE(...)                                                  \
  template <typename... Args>                                                  \
  bool accept(ATL::AInputVisitor &visitor, Args &&... args) {                  \
    bool result = true;                                                        \
    ((result = result && visitor.visit(std::forward<Args>(args))) && ...);     \
    return result;                                                             \
  }                                                                            \
  bool accept(ATL::AInputVisitor &visitor) override {                          \
    return accept(visitor, tag, __VA_ARGS__, term);                            \
  }

namespace ATL {

class AResponse : public APacket {
public:
  explicit AResponse(const char *tag) : APacket{tag} {}
  virtual bool accept(AInputVisitor &visitor) = 0;
  virtual ~AResponse() = default;
};

class AResponsePack {
  virtual bool accept(ATL::AInputVisitor &visitor) = 0;
  virtual ~AResponsePack() = default;
};

template <typename... Responses> class ResponsePack : public AResponsePack {
  static_assert((std::is_base_of<AResponse, Responses>::value && ...),
                "all response variant shall be a Response packet");

  std::variant<Responses...> variants{};

public:
  ~ResponsePack() = default;

  void accept(AInputVisitor &visitor) override {

    auto visit = [&visitor](auto &&arg) { arg.accept(visitor); };

    return std::visit(visit, variants);
  }
};

} // namespace ATL

#endif