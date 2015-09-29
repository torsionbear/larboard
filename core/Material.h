#pragma once

#include "Matrix.h"

namespace core {

class Material {
public:
	auto SetDiffuse(Vector4f) -> void;
	auto SetSpecular(Vector4f) -> void;
	auto SetEmissive(Vector4f) -> void;
	auto SetAmbientIntensity(Float32) -> void;
	auto SetShininess(Float32) -> void;
	auto SetTransparency(Float32) -> void;

	auto GetDiffuse() -> Vector4f;
	auto GetSpecular() -> Vector4f;
	auto GetEmissive() -> Vector4f;
	auto GetAmbient() -> Vector4f;
	auto GetShininess() -> Float32;
	auto GetTransparency() ->Float32;

private:
	Vector4f _diffuse;
	Vector4f _specular;
	Vector4f _emissive;
	Float32 _ambientIntensity;
	Float32 _shininess;
	Float32 _transparency;
};

}