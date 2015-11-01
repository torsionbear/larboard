#include "Bvh.h"

#include <GL/glew.h>

using std::vector;
using std::unique_ptr;
using std::make_unique;

namespace core {

core::Bvh::Bvh(vector<Shape *> && shapes) {
    _root = make_unique<BvhNode>(move(shapes));
    SubDivideBvhNode(_root.get());
}

auto Bvh::PrepareForDraw(ResourceManager & resourceManager) -> void {
    auto vertexes = vector<Vector3f>{};
    auto indexes = vector<unsigned int>{};

    auto loadRenderData = [&vertexes, &indexes](BvhNode * currentNode, int indexOffset) {
        auto minVertex = currentNode->GetAabb().GetMinVertex();
        auto maxVertex = currentNode->GetAabb().GetMaxVertex();
        vertexes.insert(vertexes.end(), {
            { minVertex(0), minVertex(1), minVertex(2) },
            { minVertex(0), maxVertex(1), minVertex(2) },
            { minVertex(0), maxVertex(1), maxVertex(2) },
            { minVertex(0), minVertex(1), maxVertex(2) },
            { maxVertex(0), minVertex(1), minVertex(2) },
            { maxVertex(0), maxVertex(1), minVertex(2) },
            { maxVertex(0), maxVertex(1), maxVertex(2) },
            { maxVertex(0), minVertex(1), maxVertex(2) },
        });
        auto index = vector<unsigned int>{
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7,
        };
        for (auto & i : index) {
            i += indexOffset;
        }
        indexes.insert(indexes.end(), index.begin(), index.end());
    };
    // traverse bvh and fill vertexes and indexes
    auto currentNode = _root.get();
    auto indexOffset = 0;
    while (currentNode != nullptr) {
        if (currentNode->_leftChild == nullptr) {
            loadRenderData(currentNode, indexOffset);
            indexOffset += 8;
            currentNode = currentNode->_rightChild.get();
        } else {
            auto preNode = currentNode->_leftChild.get();
            while (preNode->_rightChild != nullptr && preNode->_rightChild.get() != currentNode) {
                preNode = preNode->_rightChild.get();
            }
            if (preNode->_rightChild == nullptr) {
                preNode->_rightChild = unique_ptr<BvhNode>(currentNode);    // hack: now currentNode is handled by 2 unique_ptr...
                currentNode = currentNode->_leftChild.get();
            } else {
                preNode->_rightChild.release(); // hack: this unique_ptr is a duplicated one, release it.
                loadRenderData(currentNode, indexOffset);
                indexOffset += 8;
                currentNode = currentNode->_rightChild.get();
            }
        }
    }
    _vao = resourceManager.LoadBvh(vertexes, indexes);
    _indexCount = indexes.size();
    _shaderProgram.SendToCard();
}

auto Bvh::Draw() -> void {
    glBindVertexArray(_vao);
    _shaderProgram.Use();

    auto location = glGetUniformLocation(_shaderProgram.GetHandler(), "color");
    glUniform4f(location, 1.0f, 0.0f, 0.0f, 1.0f);

    glDrawElements(GL_LINES, _indexCount, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0));
}

auto Bvh::SubDivideBvhNode(BvhNode * node) -> void {
    assert(node->_leftChild == nullptr && node->_rightChild == nullptr);
    if (node->_shapes.size() <= 1u) {
        return;
    }

    // 1. calculate centers' AABB
    auto centerAabb = Aabb{};
    for (auto const& shape : node->_shapes) {
        centerAabb.Expand(shape->GetAabb().GetCenter());
    }
    // 2. calculate split point (half of centers' AABB's longest axis)
    auto axisIndex = 0;
    auto diameter = centerAabb.GetMaxVertex() - centerAabb.GetMinVertex();
    auto length = diameter(0);
    for (auto i = 1u; i < 3; ++i) {
        if (diameter(i) > length) {
            length = diameter(i);
            axisIndex = i;
        }
    }
    if (length < MaxCenterAabbRadius) { // stop splitting BvhNode if children's centers' AABB's radius is smaller than this value
        return;
    }

    auto splitPoint = centerAabb.GetMinVertex()(axisIndex) + length / 2;

    // 3. divide nodes into 2 groups according to split point
    auto group0 = std::vector<Shape *>();
    auto group1 = std::vector<Shape *>();
    for (auto * shape : node->_shapes) {
        if (shape->GetAabb().GetCenter()(axisIndex) <= splitPoint) {
            group0.push_back(shape);
        } else {
            group1.push_back(shape);
        }
    }
    node->_leftChild = make_unique<BvhNode>(move(group0));
    node->_rightChild = make_unique<BvhNode>(move(group1));
    node->_shapes.clear();

    // 4. recursively construct children
    SubDivideBvhNode(node->_leftChild.get());
    SubDivideBvhNode(node->_rightChild.get());
}

}