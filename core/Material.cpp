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
    _shaderData._diffuseEmissive(0) = diffuse(0);
    _shaderData._diffuseEmissive(1) = diffuse(1);
    _shaderData._diffuseEmissive(2) = diffuse(2);
}

auto Material::SetEmissive(Float32 emissive) -> void {
	_shaderData._diffuseEmissive(3) = emissive;
}

auto Material::SetSpecular(Vector4f specular) -> void {
    _shaderData._specularShininess(0) = specular(0);
    _shaderData._specularShininess(1) = specular(1);
    _shaderData._specularShininess(2) = specular(2);
}

auto Material::SetShininess(Float32 shininess) -> void {
	_shaderData._specularShininess(3) = shininess;
}

auto Material::SetTransparency(Float32 transparency) -> void {
	_transparency = transparency;
}

auto core::Material::GetDiffuse() -> Vector4f {
	return _shaderData._diffuseEmissive;
}

auto Material::GetEmissive() -> Float32 {
    return _shaderData._diffuseEmissive(3);
}

auto Material::GetSpecular() -> Vector4f {
	return _shaderData._specularShininess;
}

auto Material::GetShininess() -> Float32 {
	return _shaderData._specularShininess(3);
}

auto Material::GetTransparency() -> Float32 {
	return _transparency;
}

}
