#include "DirectionalLight.h"

namespace core {

DirectionalLight::DirectionalLight()
	: _color{1.0f, 1.0f, 1.0f}
	, _direction{0.0f, 0.0f, -1.0f, 0.0f} {
}

auto DirectionalLight::SetColor(Vector3f value) -> void {
	_color = value;
}

auto DirectionalLight::SetDirection(Vector4f value) -> void {
	_direction = value;
}

auto DirectionalLight::GetShaderData() const -> ShaderData {
	return ShaderData{
		Vector4f{ _color(0), _color(1), _color(2), 1.0f },
		GetMatrix() * _direction,
	};
}

}