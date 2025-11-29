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
#include "atlink/core/Enum.h"
#include "atlink/core/Types.h"

namespace ATL_NS {
namespace Core {

class ACommandVisitor {
  public:
    virtual bool visit(const Sequence &) = 0;
    virtual bool visit(const QuotedStringView) = 0;
    virtual bool visit(const AEnum &) = 0;
    virtual bool visit(int) = 0;
    virtual size_t written() const = 0;
    virtual ~ACommandVisitor() = default;
};

class AResponseVisitor {
  public:
    virtual bool visit(const Sequence &) = 0;
    virtual bool visit(QuotedStringStorage) = 0;
    virtual bool visit(LineText &) = 0;
    virtual bool visit(AEnum &) = 0;
    virtual bool visit(int &) = 0;
    virtual void rewind() = 0;
    virtual size_t consumed() const = 0;
    virtual ~AResponseVisitor() = default;
};

class APacket {
  public:
    Sequence tag;

    APacket() : tag{""} {}
    explicit APacket(ReadOnlyText tag) : tag{tag} {}
    virtual ~APacket() = default;

  protected:
    template <typename... Args>
    bool accept(AResponseVisitor &visitor, Args &&...args) {
        return acceptWithTerm(visitor, Constants::CrLf, std::forward<Args>(args)...);
    }

    template <typename... Args>
    bool accept(ACommandVisitor &visitor, Args &&...args) const {
        return acceptWithTerm(visitor, Constants::CrLf, std::forward<Args>(args)...);
    }

    template <typename... Args>
    bool acceptWithTerm(AResponseVisitor &visitor, const Sequence &term, Args &&...args) {
        if (tag.length() > 0U) {
            if (!visitor.visit(tag))
                return false;
        }
        if constexpr (sizeof...(args) > 0) {
            if (!visitWithCommas(visitor, std::forward<Args>(args)...))
                return false;
        }
        return visitor.visit(term);
    }

    template <typename... Args>
    bool acceptWithTerm(ACommandVisitor &visitor, const Sequence &term, Args &&...args) const {
        if (tag.length() > 0U) {
            if (!visitor.visit(tag))
                return false;
        }
        if constexpr (sizeof...(args) > 0) {
            if (!visitWithCommas(visitor, std::forward<Args>(args)...))
                return false;
        }
        return visitor.visit(term);
    }

  private:
    template <typename Visitor, typename First, typename... Rest>
    static bool visitWithCommas(Visitor &visitor, First &&first, Rest &&...rest) {
        if (!visitor.visit(std::forward<First>(first)))
            return false;
        if constexpr (sizeof...(rest) > 0) {
            if (!visitor.visit(Constants::Comma))
                return false;
            return visitWithCommas(visitor, std::forward<Rest>(rest)...);
        }
        return true;
    }
};

} // namespace Core
} // namespace ATL_NS