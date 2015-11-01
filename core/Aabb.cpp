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

auto Aabb::GetCenter() const -> Point4f {
    return (_minVertex + _maxVertex) * 0.5;
}

}