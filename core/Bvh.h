#pragma once

#include "BvhNode.h"
#include "ShaderProgram.h"

namespace core {

class Bvh {
    static constexpr const Float32 MaxCenterAabbRadius = 0.5f;
public:
    Bvh(std::vector<Shape *> && shapes);
public:
    auto GetRoot() const -> BvhNode * {
        return _root.get();
    }
    auto GetAabbs() const -> std::vector<Aabb *> const& {
        return _aabbs;
    }
    auto GetVao() const {
        return _vao;
    }
    auto SetVao(openglUint vao) {
        _vao = vao;
    }
    auto GetShaderProgram() -> ShaderProgram * {
        return &_shaderProgram;
    }
private:
    auto SubDivideBvhNode(BvhNode * node) -> void;
    auto GetAabbList() -> void;
private:
    std::unique_ptr<BvhNode> _root = nullptr;
    std::vector<Aabb *> _aabbs;

    openglUint _vao;
    ShaderProgram _shaderProgram = ShaderProgram{ "shader/aabb_v.shader", "shader/aabb_f.shader" };
    int _indexCount = 0; // todo: get rid of this mess
};

}