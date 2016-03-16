#include "Camera.h"

#include <cmath>

#include "GlRuntimeHelper.h"

using std::array;

namespace core {

auto Camera::ShaderData::Size() -> unsigned int {
    static unsigned int size = 0u;
    if (size == 0u) {
        size = GlRuntimeHelper::GetUboAlignedSize(sizeof(Camera::ShaderData));
    }
    return size;
}

auto Camera::GetProjectTransform() const -> Matrix4x4f const& {
    return _projectTransform;
}

auto Camera::GetProjectTransformDx() const -> Matrix4x4f const& {
    return _projectTransformDx;
}

auto Camera::GetRayTo(Vector2f windowCoordinate) const -> Ray {
    auto ret = Ray{};
    ret.origin = GetTransform() * Point4f {
        (1 - windowCoordinate(0)) * _lowerLeft(0) + windowCoordinate(0) * _upperRight(0),
            (1 - windowCoordinate(1)) * _upperRight(1) + windowCoordinate(1) * _lowerLeft(1),
            -_lowerLeft(2),
            1,
    };
    ret.direction = Normalize(static_cast<Vector4f>(ret.origin - GetPosition()));
    ret.length = _farPlane - _lowerLeft(2); // make it simple for now...
    return ret;
}

auto Camera::GetSightDistance() const -> Float32 {
    return Length(_lowerLeft) * _farPlane / _lowerLeft(2);
}

auto Camera::SetPerspective(Point4f lowerLeft, Point4f upperRight, Float32 farPlane) -> void {
    _lowerLeft = lowerLeft;
    _upperRight = upperRight;
    _farPlane = farPlane;
    // third column has been negate to transform right-hand world space to left-hand screen space.
    _projectTransform = Matrix4x4f{
        2 * lowerLeft(2) / (upperRight(0) - lowerLeft(0)), 0, (upperRight(0) + lowerLeft(0)) / (upperRight(0) - lowerLeft(0)), 0,
        0, 2 * lowerLeft(2) / (upperRight(1) - lowerLeft(1)), (upperRight(1) + lowerLeft(1)) / (upperRight(1) - lowerLeft(1)), 0,
        0, 0, -(farPlane + lowerLeft(2)) / (farPlane - lowerLeft(2)), -2 * farPlane*lowerLeft(2) / (farPlane - lowerLeft(2)),
        0, 0, -1, 0
    };
    _projectTransformDx = Matrix4x4f{
        2 * lowerLeft(2) / (upperRight(0) - lowerLeft(0)), 0, (upperRight(0) + lowerLeft(0)) / (upperRight(0) - lowerLeft(0)), 0,
        0, 2 * lowerLeft(2) / (upperRight(1) - lowerLeft(1)), (upperRight(1) + lowerLeft(1)) / (upperRight(1) - lowerLeft(1)), 0,
        0, 0, -farPlane / (farPlane - lowerLeft(2)), -farPlane*lowerLeft(2) / (farPlane - lowerLeft(2)),
        0, 0, -1, 0
    };
}

auto Camera::SetPerspective(Float32 aspectRatio, Float32 fieldOfView, Float32 nearPlane, Float32 farPlane) -> void {
    auto halfWidth = nearPlane;
    auto halfHeight = nearPlane;
    auto & majorAxis = aspectRatio > 1 ? halfWidth : halfHeight;
    auto & minorAxis = aspectRatio > 1 ? halfHeight : halfWidth;

    // according to x3d standard:
    // where the smaller of display width or display height determines which angle equals the fieldOfView 
    // (the larger angle is computed using the relationship described above). 
    // The larger angle shall not exceed ¦Ð and may force the smaller angle to be less than fieldOfView 
    // in order to sustain the aspect ratio.
    // here we keep fieldOfView less than 5/6¦Ð.
    auto maxFieldOfView = pi * 5 / 6;
    auto tangent = aspectRatio * fieldOfView >= maxFieldOfView ? tan(maxFieldOfView / 2) : tan(fieldOfView / 2);
    majorAxis *= tangent;
    minorAxis *= tangent / aspectRatio;

    SetPerspective({ -halfWidth, -halfHeight, nearPlane, 1 }, { halfWidth, halfHeight, nearPlane, 1 }, farPlane);
}

auto Camera::GetShaderData() -> ShaderData {
    return ShaderData{
        GetRigidBodyMatrixInverse(),
        _projectTransform,
        GetTransform(),
        GetPosition(),
    };
}

auto Camera::GetViewFrustumVertex() const-> array<Point4f, 8> {
    auto near = GetNearPlane();
    auto far = GetFarPlane();
    auto halfWidth = GetHalfWidth();
    auto halfHeight = GetHalfHeight();
    auto farHalfWidth = halfWidth * far / near;
    auto farHalfHeight = halfHeight * far / near;
    auto transform = GetTransform();
    auto viewFrustumVertex = array<Point4f, 8>{
        transform * Point4f{ -halfWidth, -halfHeight, -near, 1 },
            transform * Point4f{ -halfWidth, halfHeight, -near, 1 },
            transform * Point4f{ halfWidth, -halfHeight, -near, 1 },
            transform * Point4f{ halfWidth, halfHeight, -near, 1 },
            transform * Point4f{ -farHalfWidth, -farHalfHeight, -far, 1 },
            transform * Point4f{ -farHalfWidth, farHalfHeight, -far, 1 },
            transform * Point4f{ farHalfWidth, -farHalfHeight, -far, 1 },
            transform * Point4f{ farHalfWidth, farHalfHeight, -far, 1 },
    };
    return viewFrustumVertex;
}

}
