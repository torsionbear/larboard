#pragma once

#include "X3dNode.h"
#include "Shape.h"

namespace x3dParser {

class Group : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode *) -> void override;

    auto GetShape() const -> std::vector<Shape *> const&;

private:
    std::vector<Shape *> _shape;
};

}