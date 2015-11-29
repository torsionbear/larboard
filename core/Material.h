#pragma once

#include "Matrix.h"

namespace core {

class Material {
public:
	struct ShaderData {
    public:
		Vector4f _diffuseEmissive;
		Vector4f _specularShininess;
    public:
        static auto Size() -> unsigned int;
	};
public:
	auto SetDiffuse(Vector4f diffuse) -> void;
    auto SetEmissive(Float32 emissive) -> void;
	auto SetSpecular(Vector4f specular) -> void;
	auto SetShininess(Float32 shininess) -> void;
	auto SetTransparency(Float32 transparency) -> void;

	auto GetDiffuse() -> Vector4f;
    auto GetEmissive()->Float32;
	auto GetSpecular() -> Vector4f;
	auto GetShininess() -> Float32;
	auto GetTransparency() -> Float32;

	auto GetUboOffset() const -> int {
		return _uboOffset;
	}
	auto SetUboOffset(int offset) -> void {
		_uboOffset = offset;
	}
	auto GetShaderData() const -> ShaderData const& {
		return _shaderData;
	}
private:
    Float32 _transparency;
	ShaderData _shaderData;
	openglInt _uboOffset;

};

}