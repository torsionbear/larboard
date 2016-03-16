#pragma once

#include <array>

#include "Viewpoint.h"
#include "Matrix.h"
#include "Movable.h"
#include "Ray.h"

namespace core {

class Camera : public Viewpoint {
public:
    struct ShaderData {
    public:
        Matrix4x4f viewTransform;
        Matrix4x4f projectTransform;
        Matrix4x4f viewTransformInverse;
        Vector4f viewPosition;
    public:
        static auto Size() -> unsigned int;
    };
public:
    auto GetProjectTransform() const->Matrix4x4f const& override;
    auto GetProjectTransformDx() const->Matrix4x4f const& override;
public:
    auto SetPerspective(Point4f lowerLeft, Point4f upperRight, Float32 farPlane) -> void;
    auto SetPerspective(Float32 aspectRatio, Float32 fieldOfView, Float32 nearPlane, Float32 farPlane) -> void;
    auto GetRayTo(Vector2f windowCoordinate) const->Ray;
    auto GetFarPlane() const -> Float32 {
        return _farPlane;
    }
    auto GetSightDistance() const->Float32;
    auto GetNearPlane() const -> Float32 {
        return _lowerLeft(2);
    }
    auto GetHalfWidth() const -> Float32 {
        return _upperRight(0);
    }
    auto GetHalfHeight() const -> Float32 {
        return _upperRight(1);
    }

    auto GetShaderData()->ShaderData;
    auto GetUboOffset() const -> int {
        return _uboOffset;
    }
    auto SetUboOffset(int offset) -> void {
        _uboOffset = offset;
    }
    auto GetViewFrustumVertex() const->std::array<Point4f, 8>;
private:
    Matrix4x4f _projectTransform;
    Matrix4x4f _projectTransformDx;
    Point4f _lowerLeft;
    Point4f _upperRight;
    Float32 _farPlane;
    openglInt _uboOffset;
};

}