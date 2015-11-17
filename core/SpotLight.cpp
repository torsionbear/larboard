#include "SpotLight.h"

namespace core {

auto SpotLight::SetColor(Vector4f color) -> void {
	_color = color;
}

auto SpotLight::SetDirection(Vector4f direction) -> void {
	_direction = direction;
}

auto SpotLight::SetRadius(Float32 radius) -> void {
	_radius = radius;
}

auto SpotLight::SetAttenuation(Vector3f attenuation) -> void {
	_attenuation = attenuation;
}

auto SpotLight::SetBeamWidth(Float32 beamWidth) -> void {
	_beamWidth = beamWidth;
}

auto SpotLight::SetCutOffAngle(Float32 cutOffAngle) -> void {
	_cutOffAngle = cutOffAngle;
}

auto SpotLight::GetShaderData() -> ShaderData {
	return ShaderData{
		_color,
		GetPosition(),
        GetTransform() * _direction,
		Vector4f{ _attenuation(0), _attenuation(1), _attenuation(2), _radius },
		_beamWidth,
		_cutOffAngle,
		{0.0f, 0.0f},
	};
}

}