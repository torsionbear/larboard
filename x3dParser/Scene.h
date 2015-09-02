#pragma once

#include "X3dNode.h"
#include "Transform.h"

namespace x3dParser {

class Scene : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode *) -> void override;

    auto GetTransform() const -> std::vector<Transform*> const&;

private:
    std::vector<Transform *> _transform;
};

}