#pragma once

#include "X3dNode.h"
#include "Shape.h"

namespace x3dParser {

class Group : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetShape() -> std::vector<std::unique_ptr<Shape>>&;

private:
    std::vector<std::unique_ptr<Shape>> _shape;
};

}