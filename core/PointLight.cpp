#include "PointLight.h"

namespace core {

PointLight::PointLight() 
	: _color{1.0f, 1.0f, 1.0f}
	, _attenuation{1.0f, 0.0f, 0.0f}
	, _radius(10.0f) {
}

auto PointLight::SetColor(Vector3f value) -> void {
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
		GetPosition(),
		Vector4f{_color(0), _color(1), _color(2), 1.0f},
		Vector4f{_attenuation(0), _attenuation(1), _attenuation(2), _radius},
	};
}

}
