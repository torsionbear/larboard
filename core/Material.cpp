#include "Material.h"

#include "GlRuntimeHelper.h"

namespace core {

auto Material::ShaderData::Size() -> unsigned int {
    static unsigned int size = 0u;
    if (size == 0u) {
        size = GlRuntimeHelper::GetUboAlignedSize(sizeof(Material::ShaderData));
    }
    return size;
}

auto Material::SetDiffuse(Vector4f diffuse) -> void {
	_shaderData._diffuse = diffuse;
}

auto Material::SetSpecular(Vector4f specular) -> void {
	_shaderData._specular = specular;
}

auto Material::SetEmissive(Vector4f emissive) -> void {
	_shaderData._emissive = emissive;
}

auto Material::SetAmbientIntensity(Float32 ambientIntensity) -> void {
	_shaderData._ambientIntensity = ambientIntensity;
}

auto Material::SetShininess(Float32 shininess) -> void {
	_shaderData._shininess = shininess;
}

auto Material::SetTransparency(Float32 transparency) -> void {
	_shaderData._transparency = transparency;
}

auto core::Material::GetDiffuse() -> Vector4f {
	return _shaderData._diffuse;
}

auto Material::GetSpecular() -> Vector4f {
	return _shaderData._specular;
}

auto Material::GetEmissive() -> Vector4f {
	return _shaderData._emissive;
}

auto Material::GetAmbient() -> Vector4f {
	return _shaderData._diffuse * _shaderData._ambientIntensity;
}

auto Material::GetShininess() -> Float32 {
	return _shaderData._shininess;
}

auto Material::GetTransparency() -> Float32 {
	return _shaderData._transparency;
}

}
