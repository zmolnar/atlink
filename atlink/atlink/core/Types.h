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

#pragma once

namespace ATL_NS {
namespace Core {

class Tag {
    const char *str;

  public:
    explicit Tag(const char *tag) : str{tag} {}
    const char *asStr() const {
        return str;
    }
};

class Term {};

class AEnum {
  public:
    // virtual int32_t asInt() const = 0;
    virtual const char *asStr() const = 0;
    virtual bool fromStr(const char *str) = 0;
    virtual ~AEnum() = default;
};

} // namespace Core
} // namespace ATL_NS
