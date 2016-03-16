#include "Aabb.h"

#include <limits>

using std::numeric_limits;
using std::vector;
using std::array;

namespace core {

Aabb::Aabb()
    : _minVertex{ numeric_limits<Float32>::max(), numeric_limits<Float32>::max(), numeric_limits<Float32>::max(), 1.0f }
    , _maxVertex{ numeric_limits<Float32>::lowest(), numeric_limits<Float32>::lowest(), numeric_limits<Float32>::lowest(), 1.0f } {
}

Aabb::Aabb(std::initializer_list<Point4f> points)
    : Aabb() {
    for (auto const& point : points) {
        Expand(point);
    }
}

auto Aabb::Expand(Aabb const & other) -> void {
    Expand(other.GetMinVertex());
    Expand(other.GetMaxVertex());
}

auto Aabb::Expand(Point4f vertex) -> void {
    for (auto i = 0; i < 3; ++i) {
        if (vertex(i) < _minVertex(i)) {
            _minVertex(i) = vertex(i);
        }
        if (vertex(i) > _maxVertex(i)) {
            _maxVertex(i) = vertex(i);
        }
    }
}

auto Aabb::Intersect(Aabb const & other) -> void {
    for (auto i = 0; i < 3; ++i) {
        _minVertex(i) = std::max(_minVertex(i), other._minVertex(i));
        _maxVertex(i) = std::min(_maxVertex(i), other._maxVertex(i));
    }
}

auto Aabb::GetCenter() const -> Point4f {
    return (_minVertex + _maxVertex) * 0.5;
}
// ray-AABB intersection algorithm, see "Real-time rendering" Ch16.7.
auto Aabb::IntersectRay(Ray ray) const -> Float32 {
    auto tMin = std::numeric_limits<Float32>::lowest();
    auto tMax = std::numeric_limits<Float32>::max();
    auto axes = std::array<Vector4f, 3>{ Vector4f{ 1, 0, 0, 0 }, Vector4f{ 0, 1, 0, 0 }, Vector4f{ 0, 0, 1, 0 }};
    for (auto axisIndex = 0; axisIndex < 3; ++axisIndex) {
        auto cosine = DotProduct(ray.direction, axes[axisIndex]);
        if (abs(cosine) > std::numeric_limits<Float32>::epsilon()) {
            auto t1 = (_minVertex(axisIndex) - ray.origin(axisIndex)) / cosine;
            auto t2 = (_maxVertex(axisIndex) - ray.origin(axisIndex)) / cosine;
            if (t1 > t2) {
                std::swap(t1, t2);
            }
            if (t1 > tMin) {
                tMin = t1;
            }
            if (t2 < tMax) {
                tMax = t2;
            }
            if (tMax < 0 || tMin > tMax) {
                return -1;
            }
        } else if (_minVertex(axisIndex) - ray.origin(axisIndex) > 0 || _maxVertex(axisIndex) - ray.origin(axisIndex) < 0) {
            // ray is parallel with axis and reside outside of 2 bounding plane
            return -1;
        }
        if (tMin >= ray.length) { // intersection point is farther than ray length
            return -1;
        }
    }
    return tMin > 0 ? tMin : tMax;
}

}