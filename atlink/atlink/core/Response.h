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

#include <type_traits>
#include <variant>

#include <atlink/core/Packet.h>

namespace ATL_NS {
namespace Core {

class Line : public APacket {
  public:
    Line() : APacket{} {}
    explicit Line(const char *tag) : APacket{tag} {}
    virtual bool accept(AResponseVisitor &visitor) = 0;

  protected:
    template <typename... Args>
    bool acceptImpl(AResponseVisitor &visitor, Args &&...args) {
        if (0U < tag.length()) {
            (void)visitor.visit(tag);
        }
        return APacket::accept(visitor, std::forward<Args>(args)...);
    }
};

class Response : public APacket {
  public:
    explicit Response(const char *tag) : APacket{tag} {}
    virtual bool accept(AResponseVisitor &visitor) = 0;
    virtual ~Response() = default;

  protected:
    template <typename... Args>
    bool acceptImpl(AResponseVisitor &visitor, Args &&...args) {
        (void)visitor.visit(Constants::CrLf);
        return APacket::acceptWithTerm(visitor, Constants::CrLf, std::forward<Args>(args)...);
    }
};

class MultiLineResponse : public Response {
  public:
    explicit MultiLineResponse(const char *tag) : Response{tag} {}
    virtual bool accept(AResponseVisitor &visitor) override = 0;
    virtual ~MultiLineResponse() = default;

  protected:
    template <typename... LineTs>
    bool acceptImpl(AResponseVisitor &visitor, LineTs &&...lines) {
        static_assert((std::is_base_of<Line, std::remove_reference_t<LineTs>>::value && ...),
                      "MultiLineResponse::acceptImpl expects all LineTs to derive from Line");
        (void)visitor.visit(Constants::CrLf);
        if (0U < tag.length()) {
            if (!visitor.visit(tag)) {
                return false;
            }
        }
        (void)visitor.visit(Constants::CrLf);
        return (lines.accept(visitor) && ...);
    }
};

} // namespace Core
} // namespace ATL_NS
