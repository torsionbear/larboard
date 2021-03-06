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
    Aabb(std::initializer_list<Point4f> points);
public:
    auto Expand(Aabb const& other) -> void;
    auto Expand(Point4f vertex) -> void;
    auto Intersect(Aabb const& other) -> void;
    auto GetCenter() const -> Point4f;
    auto GetMinVertex() const -> Point4f {
        return _minVertex;
    }
    auto SetMinVertex(Point4f minVertex) -> void {
        _minVertex = minVertex;
    }
    auto GetMaxVertex() const -> Point4f {
        return _maxVertex;
    }
    auto SetMaxVertex(Point4f maxVertex) -> void {
        _maxVertex = maxVertex;
    }
    auto GetVertex() const -> std::array<Point4f, 8> {
        return std::array<Point4f, 8> {
            _minVertex,
            Point4f{ _maxVertex(0), _minVertex(1), _minVertex(2), 1},
            Point4f{ _minVertex(0), _maxVertex(1), _minVertex(2), 1},
            Point4f{ _minVertex(0), _minVertex(1), _maxVertex(2), 1},
            Point4f{ _minVertex(0), _maxVertex(1), _maxVertex(2), 1},
            Point4f{ _maxVertex(0), _minVertex(1), _maxVertex(2), 1},
            Point4f{ _maxVertex(0), _maxVertex(1), _minVertex(2), 1},
            _maxVertex,
        };
    }
    auto Translate(Vector4f translate) -> Aabb {
        return Aabb{ _minVertex + translate, _maxVertex + translate };
    }
    auto IntersectRay(Ray ray) const -> Float32;
    auto IntersectTriangle(std::array<Point4f, 3> const& vertex) const -> bool;
    auto IntersectAabb(Aabb const& other) -> bool;
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
    auto Project(Point4f axis, Point4f const* points, unsigned int count, Float32 & min, Float32 & max) const -> void;
private:
    Point4f _minVertex;
    Point4f _maxVertex;

    RenderData _renderData;
    ShaderProgram * _shaderProgram;
};

}