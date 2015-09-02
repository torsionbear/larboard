#pragma once

#include "X3dNode.h"
#include "BasicType.h"

namespace x3dParser {

class Normal : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode *) -> void override;

    auto GetVector() const -> std::vector<Float3> const&;

private:
    auto SetVector(std::string&&) -> void;

private:
    std::vector<Float3> _vector;
};

}