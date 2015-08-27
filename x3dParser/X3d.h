#pragma once

#include "X3dNode.h"
#include "Scene.h"

namespace x3dParser {

class X3d : public X3dNode {
public:
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetScene() -> std::unique_ptr<Scene>&;

private:
    std::unique_ptr<Scene> _scene = nullptr;
};

}