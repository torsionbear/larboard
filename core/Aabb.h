#pragma once

#include "Primitive.h"
#include "Matrix.h"
#include "Vertex.h"
#include "Ray.h"

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
    auto IntersectRay(Ray ray) const -> Float32;
private:
    Point4f _minVertex;
    Point4f _maxVertex;
};

}