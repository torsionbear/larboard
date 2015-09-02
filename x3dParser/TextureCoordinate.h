#pragma once

#include "X3dNode.h"
#include "BasicType.h"

namespace x3dParser {

class TextureCoordinate : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode * child) -> void override;

    auto GetPoint() const -> const std::vector<Float2>&;
    auto StealPoint() -> std::vector<Float2>;

private:
    auto SetPoint(std::string&&) -> void;

private:
    std::vector<Float2> _point;
};

}