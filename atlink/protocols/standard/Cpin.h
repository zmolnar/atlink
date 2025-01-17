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

#ifndef CPIN_H
#define CPIN_H

#include <core/Command.h>
#include <core/Response.h>

namespace ATL {
namespace Proto {
namespace Std {

class CpinRead : public ATL::ACommand {
public:
  CpinRead() : ATL::ACommand("+CPIN? ") {}
  REFLECT_COMMAND_NONE();
};

class CpinWrite : public ATL::ACommand {
public:
  CpinWrite() : ATL::ACommand("+CPIN=") {}
  int pin;
  REFLECT_COMMAND(pin);
};

class CpinReadResponse : public ATL::AResponse {
public:
  enum class Code {
    READY,
    SIM_PIN,
    SIM_PUK,
    PH_SIM_PIN,
    PH_SIM_PUK,
    SIM_PIN2,
    SIM_PUK2,
  };

  ATL::Enum<Code, 7U> code{{
      {Code::READY, "READY"},
      {Code::SIM_PIN, "SIM PIN"},
      {Code::SIM_PUK, "SIM PUK"},
      {Code::PH_SIM_PIN, "PH_SIM PIN"},
      {Code::PH_SIM_PUK, "PH_SIM PUK"},
      {Code::SIM_PIN, "SIM PIN2"},
      {Code::SIM_PUK, "SIM PUK2"},
  }};

  CpinReadResponse() : ATL::AResponse("+CPIN: ") {}
  ~CpinReadResponse() = default;
  REFLECT_RESPONSE(code);
};

} // namespace Std
} // namespace Proto
} // namespace ATL

#endif // CPIN_H