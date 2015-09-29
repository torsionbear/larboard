#include "Material.h"

namespace core {

auto Material::SetDiffuse(Vector4f diffuse) -> void {
	_diffuse = diffuse;
}

auto Material::SetSpecular(Vector4f specular) -> void {
	_specular = specular;
}

auto Material::SetEmissive(Vector4f emissive) -> void {
	_emissive = emissive;
}

auto Material::SetAmbientIntensity(Float32 ambientIntensity) -> void {
	_ambientIntensity = ambientIntensity;
}

auto Material::SetShininess(Float32 shininess) -> void {
	_shininess = shininess;
}

auto Material::SetTransparency(Float32 transparency) -> void {
	_transparency = transparency;
}

auto core::Material::GetDiffuse() -> Vector4f {
	return _diffuse;
}

auto Material::GetSpecular() -> Vector4f {
	return _specular;
}

auto Material::GetEmissive() -> Vector4f {
	return _emissive;
}

auto Material::GetAmbient() -> Vector4f {
	return _diffuse * _ambientIntensity;
}

auto Material::GetShininess() -> Float32 {
	return _shininess;
}

auto Material::GetTransparency() -> Float32 {
	return _transparency;
}

}
