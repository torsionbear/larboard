#pragma once

#include "X3dNode.h"
#include "BasicType.h"

namespace x3dParser {

class Normal : public X3dNode {
public:
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetVector() -> std::vector<Float3>&;
    auto StealVector() -> std::vector<Float3>;

private:
    auto SetVector(std::string&&) -> void;

private:
    std::vector<Float3> _vector;
};

}