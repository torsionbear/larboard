#pragma once

#include "Movable.h"

namespace core {

class SpotLight : public Movable{
public:
	struct ShaderData {
		Vector4f color;
		Vector4f position;
		Vector4f direction;
		Vector4f attenuation;
		Float32 beamWidth;
		Float32 cutOffAngle;
		Float32 pad[2];
	};
public:
	auto SetColor(Vector4f color) -> void;
	auto SetDirection(Vector4f direction) -> void;
	auto SetRadius(Float32 radius) -> void;
	auto SetAttenuation(Vector3f attenuation) -> void;
	auto SetBeamWidth(Float32 beamWidth) -> void;
	auto SetCutOffAngle(Float32 cutOffAngle) -> void;

	auto GetShaderData() -> ShaderData;
private:
	Vector4f _color;
	Vector4f _direction;
	Vector3f _attenuation;
	Float32 _radius;
	Float32 _beamWidth;
	Float32 _cutOffAngle;
};

}