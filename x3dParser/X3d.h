#pragma once

#include "X3dNode.h"
#include "Scene.h"

namespace x3dParser {

class X3d : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode * child) -> void override;

    auto GetScene() const -> Scene const*;

private:
    Scene * _scene = nullptr;
};

}