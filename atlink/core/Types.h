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

#ifndef TYPES_H
#define TYPES_H

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <utility>

namespace ATL {

class Tag {
  const char *str;

public:
  explicit Tag(const char *tag) : str{tag} {}
  const char *asStr() const { return str; }
};

class Term {};

class AEnum {
public:
  virtual int asInt() const = 0;
  virtual const char *asStr() const = 0;
  virtual ~AEnum() = default;
};

template <typename T, size_t N> class Enum : public AEnum {

public:
  using Variant = std::pair<int, const char *>;
  using Variants = std::array<Variant, N>;

  T variant;
  Variants variants{};

  explicit Enum(
      const std::initializer_list<std::pair<T, const char *>> &values) {
    assert(N == values.size());
    size_t i = 0U;
    for (auto &v : values) {
      variants[i++] = {static_cast<int>(v.first), v.second};
    }
  }

  T get() const { return variant; }
  void set(T v) { variant = v; }

  int asInt() const override {
    auto it = find();
    assert(it != variants.end());
    return it->first;
  }

  const char *asStr() const override {
    auto it = find();
    assert(it != variants.end());
    return it->second;
  }

  typename Variants::const_iterator find() const {
    auto match = [this](typename Variants::const_reference item) {
      return static_cast<int>(variant) == item.first;
    };
    return std::find_if(variants.begin(), variants.end(), match);
  }
};

} // namespace ATL

#endif // TYPES_H