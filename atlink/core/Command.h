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

#ifndef COMMAND_H
#define COMMAND_H

#include "Packet.h"

#define REFLECT_COMMAND_NONE()                                                 \
  template <typename... Args>                                                  \
  void accept(ATL::AOutputVisitor &visitor, Args &&... args) const {           \
    (visitor.visit(args), ...);                                                \
  }                                                                            \
  void accept(ATL::AOutputVisitor &visitor) const override {                   \
    accept(visitor, tag, term);                                                \
  }

#define REFLECT_COMMAND(...)                                                   \
  template <typename... Args>                                                  \
  void accept(ATL::AOutputVisitor &visitor, Args &&... args) const {           \
    (visitor.visit(args), ...);                                                \
  }                                                                            \
  void accept(ATL::AOutputVisitor &visitor) const override {                   \
    accept(visitor, tag, __VA_ARGS__, term);                                   \
  }

namespace ATL {

class ACommand : public APacket {
public:
  explicit ACommand(const char *tag) : APacket{tag} {};
  virtual void accept(AOutputVisitor &visitor) const = 0;
  virtual ~ACommand() = default;
};

} // namespace ATL

#endif