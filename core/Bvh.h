#pragma once

#include "BvhNode.h"
#include "ResourceManager.h"
#include "ShaderProgram.h"

namespace core {

class Bvh {
    static constexpr const Float32 MaxCenterAabbRadius = 0.5f;
public:
    Bvh(std::vector<Shape *> && shapes);
public:
    auto PrepareForDraw(ResourceManager & resourceManager) -> void;
    auto Draw() -> void;
private:
    auto SubDivideBvhNode(BvhNode * node) -> void;
private:
    std::unique_ptr<BvhNode> _root = nullptr;
    openglUint _vao;
    ShaderProgram _shaderProgram = ShaderProgram{ "shader/aabb.vert", "shader/aabb.frag" };
    int _indexCount = 0; // todo: get rid of this mess
};

}