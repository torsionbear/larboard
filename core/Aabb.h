#pragma once

#include "Primitive.h"
#include "Matrix.h"
#include "Vertex.h"

namespace core {

class Aabb {
public:
    Aabb();
public:
    auto Expand(Aabb const& other) -> void;
    auto Expand(Point4f vertex) -> void;
    auto GetCenter() const -> Point4f;
    auto GetMinVertex() const -> Point4f {
        return _minVertex;
    }
    auto GetMaxVertex() const -> Point4f {
        return _maxVertex;
    }
private:
    Point4f _minVertex;
    Point4f _maxVertex;
};

}