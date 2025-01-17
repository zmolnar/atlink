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

#include "Response.h"

#include <variant>

#ifndef URC_H
#define URC_H

namespace ATL {


template <typename... Urcs> class UrcParser {
  static_assert((std::is_base_of<AResponse, Urcs>::value && ...),
                "all URC shall be a Response packet");

  std::variant<Urcs...> variants{};

public:
  UrcParser() = default;

  std::variant<Urcs...> accept(ATL::AInputVisitor& visitor) {
    return std::visit(
        [&visitor](auto &&arg) -> std::variant<Urcs...> {
          arg.accept(visitor);
          return std::variant<Urcs...>{};
        },
        variants);
  }
};

} // namespace ATL

#endif