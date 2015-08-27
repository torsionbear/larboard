#pragma once

#include "X3dNode.h"
#include "Transform.h"

namespace x3dParser {

class Scene : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetTransform() -> std::vector<std::unique_ptr<Transform>>&;

private:
    std::vector<std::unique_ptr<Transform>> _transform;
};

}