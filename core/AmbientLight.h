#pragma once

#include "Matrix.h"

namespace core {

class AmbientLight {
public:
    struct ShaderData {
        Vector4f color;
    };
public:
    auto SetColor(Vector4f color) -> void {
        _color = color;
    }
    auto GetColor() -> Vector4f {
        return _color;
    }
    auto GetShaderData()->ShaderData {
        return ShaderData{ _color };
    }
private:
    Vector4f _color;

};
}