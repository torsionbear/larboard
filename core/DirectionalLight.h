#pragma once

#include "Movable.h"

namespace core {

class DirectionalLight : public Movable {
public:
	struct ShaderData {
		Vector4f direction;
		Vector4f color;
	};
public:
	DirectionalLight();
public:
	auto SetColor(Vector3f value) -> void;
	auto SetDirection(Vector4f value) -> void;
	auto GetShaderData() const->ShaderData;
private:
	Vector3f _color;
	Vector4f _direction;
};

}