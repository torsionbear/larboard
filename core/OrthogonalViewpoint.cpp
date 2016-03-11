#include "OrthogonalViewpoint.h"

namespace core {

auto OrthogonalViewpoint::SetViewVolume(Aabb const& aabb) -> void {
    // default view direction is (0, 0, -1)
    auto minVertex = aabb.GetMinVertex();
    auto maxVertex = aabb.GetMaxVertex();
    auto left = minVertex(0);
    auto bottom = minVertex(1);
    auto near = minVertex(2);
    auto right = maxVertex(0);
    auto top = maxVertex(1);
    auto far = maxVertex(2);
    _projectTransformDx = Matrix4x4f{
        2.0f / (right - left), 0, 0, -(right + left) / (right - left),
        0, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
        0, 0, 1.0f / (far - near), -near / (far - near),
        0, 0, 0, 1
    };
}

}