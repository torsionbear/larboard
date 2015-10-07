#pragma once

#include <string>

#include "BasicType.h"
#include "X3dNode.h"

namespace x3dParser {

class PointLight : public X3dNode {
public:
public:
	auto SetAttribute(std::string const&, std::string&&) -> void override;
	auto AddChild(X3dNode *) -> void override;

	auto GetAmbientIntensity() const -> Float;
	auto GetColor() const -> Float3;
	auto GetIntensity() const ->Float;
	auto GetRadius() const ->Float;
	auto GetLocation() const -> Float3;
	auto GetAttenuation() const -> Float3;

private:
	auto SetAmbientIntensity(std::string && s) -> void;
	auto SetColor(std::string && s) -> void;
	auto SetIntensity(std::string && s) -> void;
	auto SetRadius(std::string && s) -> void;
	auto SetLocation(std::string && s) -> void;
	auto SetAttenuation(std::string && s) -> void;

private:
	Float _ambientIntensity;
	Float3 _color;
	Float _intensity;
	Float _radius;
	Float3 _location;
	Float3 _attenuation;
};

}