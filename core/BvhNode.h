#pragma once

#include <memory>
#include <array>

#include "Aabb.h"
#include "Shape.h"

namespace core {

class BvhNode {
public:
    friend class Bvh;
public:
    BvhNode() = default;
    explicit BvhNode(std::vector<Shape *> && shapes);
public:
    auto GetAabb() const -> Aabb const& {
        return _aabb;
    }
private:
    Aabb _aabb;
    std::unique_ptr<BvhNode> _leftChild = nullptr;
    std::unique_ptr<BvhNode> _rightChild = nullptr;
    std::vector<Shape *> _shapes;
};

}