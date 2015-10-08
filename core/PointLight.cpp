#include "PointLight.h"

namespace core {

auto PointLight::SetColor(Vector4f value) -> void {
	_color = value;
}

auto PointLight::SetRadius(Float32 value) -> void {
	_radius = value;
}

auto PointLight::SetAttenuation(Vector3f value) -> void {
	_attenuation = value;
}

auto PointLight::GetShaderData() const -> ShaderData {
	return ShaderData{
		_color,
		GetPosition(),
		Vector4f{_attenuation(0), _attenuation(1), _attenuation(2), _radius},
	};
}

}
