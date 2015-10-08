#pragma once

#include "Matrix.h"
#include "Movable.h"

namespace core {

class PointLight : public Movable {
public:
	struct ShaderData {
		Vector4f color;
		Vector4f position;
		Vector4f attenuation;
	};
public:
	auto SetColor(Vector4f value) -> void;
	auto SetRadius(Float32 value) -> void;
	auto SetAttenuation(Vector3f value) -> void;

	auto GetShaderData() const -> ShaderData;

private:
	Vector4f _color;
	Vector3f _attenuation;
	Float32 _radius;
};

}