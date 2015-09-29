#pragma once

#include "Matrix.h"

namespace core {

class Material {
public:
	auto SetDiffuse(Vector3f) -> void;
	auto SetSpecular(Vector3f) -> void;
	auto SetEmissive(Vector3f) -> void;
	auto SetAmbientIntensity(Float32) -> void;
	auto SetShininess(Float32) -> void;
	auto SetTransparency(Float32) -> void;

	auto GetDiffuse() -> Vector3f;
	auto GetSpecular() -> Vector3f;
	auto GetEmissive() -> Vector3f;
	auto GetAmbient() ->Vector3f;
	auto GetShininess() -> Float32;
	auto GetTransparency() ->Float32;

private:
	Vector3f _diffuse;
	Vector3f _specular;
	Vector3f _emissive;
	Float32 _ambientIntensity;
	Float32 _shininess;
	Float32 _transparency;
};

}