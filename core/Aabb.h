#pragma once

#include "Primitive.h"
#include "Matrix.h"
#include "Vertex.h"
#include "Ray.h"
#include "ShaderProgram.h"

namespace core {

class Aabb {
public:
    struct RenderData {
        openglUint _vao;
        openglUint _indexOffset;
        openglInt _baseVertex;
    };
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
    auto GetRenderData() const -> RenderData const& {
        return _renderData;
    }
    auto SetRenderData(RenderData renderData) -> void {
        _renderData = renderData;
    }
    auto GetShaderProgram() const -> ShaderProgram const* {
        return _shaderProgram;
    }
    auto SetShaderProgram(ShaderProgram * shaderProgram) -> void {
        _shaderProgram = shaderProgram;
    }
private:
    Point4f _minVertex;
    Point4f _maxVertex;

    RenderData _renderData;
    ShaderProgram * _shaderProgram;
};

}