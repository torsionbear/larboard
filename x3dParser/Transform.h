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
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetTranslation() const -> Float3;
    auto GetScale() const -> Float3;
    auto GetRotation() const -> Float4;
    auto GetTransform() -> std::vector<std::unique_ptr<Transform>>&;
    auto GetGroup() -> std::unique_ptr<Group>&;
    auto GetViewpoint() -> std::unique_ptr<Viewpoint>&;
	auto GetPointLight()->std::unique_ptr<PointLight>&;

private:
    auto SetTranslation(std::string&&) -> void;
    auto SetScale(std::string&&) -> void;
    auto SetRotation(std::string&&) -> void;

private:
    Float3 _translation;
    Float3 _scale;
    Float4 _rotation;
    std::vector<std::unique_ptr<Transform>> _transform;
    std::unique_ptr<Group> _group = nullptr;
    std::unique_ptr<Viewpoint> _viewpoint = nullptr;
	std::unique_ptr<PointLight> _pointLight = nullptr;
};

}