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

auto Aabb::IntersectTriangle(std::array<Point4f, 3> const& vertex) const -> bool {
    // aabb normal
    auto aabbNormals = std::array<Vector4f, 3>{
        Vector4f{ 1, 0, 0, 0 },
            Vector4f{ 0, 1, 0, 0 },
            Vector4f{ 0, 0, 1, 0 },
    };
    auto triangleMin = 0.0f;
    auto triangleMax = 0.0f;
    for (auto i = 0u; i < 3; ++i) {
        auto const& normal = aabbNormals[i];
        Project(normal, vertex.data(), vertex.size(), triangleMin, triangleMax);
        if (triangleMax < _minVertex(i) || triangleMin > _maxVertex(i)) {
            return false;
        }
    }

    // triangle normal
    auto triangleNormal = static_cast<Vector4f>(CrossProduct(vertex[0], vertex[1]));
    auto triangleOffset = DotProduct(triangleNormal, vertex[0]);
    auto aabbVertex = GetVertex();
    auto aabbMin = 0.0f;
    auto aabbMax = 0.0f;
    Project(triangleNormal, aabbVertex.data(), aabbVertex.size(), aabbMin, aabbMax);
    if (aabbMax < triangleOffset || aabbMin > triangleOffset) {
        return false;
    }

    // nine edge cross-products
    auto triangleEdges = std::array<Vector4f, 3> {
        vertex[0] - vertex[1],
        vertex[1] - vertex[2],
        vertex[2] - vertex[0],
    };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            // aabb normals are the same as it's edge tangents
            auto axis = CrossProduct(triangleEdges[i], aabbNormals[j]);
            Project(axis, aabbVertex.data(), aabbVertex.size(), aabbMin, aabbMax);
            Project(axis, vertex.data(), vertex.size(), triangleMin, triangleMax);
            if (aabbMax < triangleMin || aabbMin > triangleMax)
                return false;
        }
    }
    return true;
}

auto Aabb::IntersectAabb(Aabb const & other) -> bool {
    for (auto i = 0u; i < 3; ++i) {
        if (other.GetMaxVertex()(i) < _minVertex(i) || other.GetMinVertex()(i) > _maxVertex(i)) {
            return false;
        }
    }
    return true;
}

auto Aabb::Project(Point4f axis, Point4f const * points, unsigned int count, Float32 & min, Float32 & max) const -> void {
    min = std::numeric_limits<Float32>::max();
    max = std::numeric_limits<Float32>::lowest();
    for (auto i = 0u; i < count; ++i) {
        auto val = DotProduct(axis, points[i]);
        if (val < min) {
            min = val;
        }
        if (val > max) {
            max = val;
        }
    }
}

}