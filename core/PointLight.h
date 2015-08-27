#pragma once

#include "Primitive.h"
#include "Movable.h"

namespace core {

class PointLight : public Movable {
public:
	auto SetAmbientIntensity(Float32) -> void;
	auto SetColor(Vector3f) -> void;
	auto SetIntensity(Float32) -> void;
	auto SetRadius(Float32) -> void;
	auto SetLocation(Vector3f) -> void;

private:
	Float32 _ambientIntensity;
	Vector3f _color;
	Float32 _intensity;
	Float32 _radius;
	Vector3f _location;
};

}