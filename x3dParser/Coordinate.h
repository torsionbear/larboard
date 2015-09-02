#pragma once

#include "X3dNode.h"
#include "BasicType.h"

namespace x3dParser {

class Coordinate : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode *) -> void override;

    auto GetPoint() const -> std::vector<Float3> const&;

private:
    auto SetPoint(std::string&&) -> void;

private:
    std::vector<Float3> _point;
};

}