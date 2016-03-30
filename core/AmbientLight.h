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
    auto SetRenderDataId(unsigned int id) {
        _renderDataId = id;
    }
    auto GetRenderDataId() const -> unsigned int {
        return _renderDataId;
    }
private:
    Vector4f _color;
    unsigned int _renderDataId;
};
}