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
    auto GetAabb() -> Aabb & {
        return _aabb;
    }
    auto IsLeaf() const -> bool {
        return _leftChild == nullptr && _rightChild == nullptr;
    }
    auto LeftChild() const -> BvhNode * {
        return _leftChild.get();
    }
    auto RightChild() const -> BvhNode * {
        return _rightChild.get();
    }
    auto GetShapes() const -> std::vector<Shape *> const& {
        return _shapes;
    }
private:
    Aabb _aabb;
    std::unique_ptr<BvhNode> _leftChild = nullptr;
    std::unique_ptr<BvhNode> _rightChild = nullptr;
    std::vector<Shape *> _shapes;
};

}