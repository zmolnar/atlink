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

#include "atlink/core/Constants.h"
#include "atlink/core/Types.h"

namespace ATL_NS {
namespace Core {

class AOutputVisitor {
  public:
    virtual void visit(const Tag &tag) = 0;
    virtual void visit(const Comma &comma) = 0;
    virtual void visit(const Term &term) = 0;
    virtual void visit(const ReadOnlyText str) = 0;
    virtual void visit(const AEnum &e) = 0;
    virtual void visit(int i) = 0;
    virtual ~AOutputVisitor() = default;
};

class AInputVisitor {
  public:
    virtual void visit(const Tag &tag) = 0;
    virtual void visit(const Comma &comma) = 0;
    virtual void visit(const Term &term) = 0;
    virtual void visit(MutableBuffer &str) = 0;
    virtual void visit(AEnum &e) = 0;
    virtual void visit(int &i) = 0;
    virtual ~AInputVisitor() = default;
};

class APacket {
  public:
    Tag tag;

    explicit APacket(ReadOnlyText tag) : tag{tag} {}
    virtual ~APacket() = default;

  protected:
    template <typename... Args>
    void accept(AInputVisitor &visitor, Args &&...args) {
        visitor.visit(tag);
        if constexpr (sizeof...(args) > 0) {
            visitWithCommas(visitor, std::forward<Args>(args)...);
        }
        visitor.visit(Constants::TERM);
    }

    template <typename... Args>
    void accept(AOutputVisitor &visitor, Args &&...args) const {
        visitor.visit(tag);
        if constexpr (sizeof...(args) > 0) {
            visitWithCommas(visitor, std::forward<Args>(args)...);
        }
        visitor.visit(Constants::TERM);
    }

  private:
    template <typename Visitor, typename First, typename... Rest>
    static void visitWithCommas(Visitor &visitor, First &&first, Rest &&...rest) {
        visitor.visit(std::forward<First>(first));
        if constexpr (sizeof...(rest) > 0) {
            visitor.visit(Constants::COMMA);
            visitWithCommas(visitor, std::forward<Rest>(rest)...);
        }
    }
};

} // namespace Core
} // namespace ATL_NS
