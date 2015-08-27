#pragma once

#include "X3dNode.h"
#include "BasicType.h"

namespace x3dParser {

class Coordinate : public X3dNode {
public:
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetPoint() -> std::vector<Float3>&;

private:
    auto SetPoint(std::string&&) -> void;

private:
    std::vector<Float3> _point;
};

}