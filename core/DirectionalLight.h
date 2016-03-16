#pragma once

#include <limits>

#include "Camera.h"
#include "OrthogonalViewpoint.h"
#include "Aabb.h"

namespace core {

class DirectionalLight : public OrthogonalViewpoint {
public:
    struct ShaderData {
        Vector4f color;
        Vector4f direction;
    };
public:
    DirectionalLight()
        : _color{ 1.0f, 1.0f, 1.0f, 1.0f }
        , _direction{ 0.0f, 0.0f, -1.0f, 0.0f } {
    }
public:
    auto SetColor(Vector3f value) -> void {
        _color = Vector4f{ value(0), value(1), value(2), 1.0f };
    }
    auto SetDirection(Vector4f value) -> void {
        _direction = value;
    }
    auto GetColor() -> Vector4f {
        return _color;
    }
    auto GetDirection() -> Vector4f {
        return GetTransform() * _direction;
    }
    auto GetShaderData() const -> ShaderData {
        return ShaderData{
            _color,
            GetTransform() * _direction,
        };
    }
    auto ComputeShadowMappingVolume(Camera* camera, Aabb shadowCasterAabb) -> void;
private:
    Vector4f _color;
    Vector4f _direction;
};

}