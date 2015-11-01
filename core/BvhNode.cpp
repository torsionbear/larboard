#include "BvhNode.h"

using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::array;

namespace core {

BvhNode::BvhNode(std::vector<Shape *> && shapes)
    : _shapes(move(shapes)) {
    _aabb = Aabb{};
    for (auto const& shape : _shapes) {
        _aabb.Expand(shape->GetAabb());
    }
}

}
