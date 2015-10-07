#pragma once

#include <string>

#include "BasicType.h"
#include "X3dNode.h"

namespace x3dParser {
class DirectionalLight : public X3dNode {
public:
	auto SetAttribute(std::string const& attribute, std::string&& value) -> void override;
	auto AddChild(X3dNode *) -> void override;

	auto GetAmbientIntensity() const -> Float;
	auto GetColor() const -> Float3;
	auto GetIntensity() const -> Float;
	auto GetDirection() const -> Float3;

private:
	auto SetAmbientIntensity(std::string && s) -> void;
	auto SetColor(std::string && s) -> void;
	auto SetIntensity(std::string && s) -> void;
	auto SetDirection(std::string && s) -> void;

private:
	Float _ambientIntensity;
	Float3 _color;
	Float _intensity;
	Float3 _direction;
};

}