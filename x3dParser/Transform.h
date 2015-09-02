#pragma once

#include <string>

#include "X3dNode.h"
#include "BasicType.h"
#include "Group.h"
#include "Viewpoint.h"
#include "PointLight.h"

namespace x3dParser {

class Transform : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode * child) -> void override;

    auto GetTranslation() const -> Float3;
    auto GetScale() const -> Float3;
    auto GetRotation() const -> Float4;
    auto GetTransform() const -> std::vector<Transform *> const&;
    auto GetGroup() const-> Group const*;
    auto GetViewpoint() const -> Viewpoint const*;
	auto GetPointLight() const -> PointLight const*;

private:
    auto SetTranslation(std::string&&) -> void;
    auto SetScale(std::string&&) -> void;
    auto SetRotation(std::string&&) -> void;

private:
    Float3 _translation;
    Float3 _scale;
    Float4 _rotation;
    std::vector<Transform *> _transform;
    Group * _group = nullptr;
    Viewpoint * _viewpoint = nullptr;
	PointLight * _pointLight = nullptr;
};

}