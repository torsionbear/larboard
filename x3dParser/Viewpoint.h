#pragma once

#include "X3dNode.h"
#include "BasicType.h"

namespace x3dParser {

class Viewpoint : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode *) -> void override;

    auto GetCenterOfRotation() const -> Float3;
    auto GetPosition() const -> Float3;
    auto GetOrientation() const -> Float4;
    auto GetFieldOfView() const -> Float;

private:
    auto SetCenterOfRotation(std::string&&) -> void;
    auto SetPosition(std::string&&) -> void;
    auto SetOrientation(std::string&&) -> void;
    auto SetFieldOfView(std::string&&) -> void;

private:
    Float3 _centerOfRotation;
    Float3 _position;
    Float4 _orientation;
    Float _fieldOfView;
};

}