#include "PointLight.h"

namespace core {

auto core::PointLight::SetAmbientIntensity(Float32 value) -> void {
	_ambientIntensity = value;
}

auto PointLight::SetColor(Vector3f value) -> void {
	_color = value;
}

auto PointLight::SetIntensity(Float32 value) -> void {
	_intensity = value;
}

auto PointLight::SetRadius(Float32 value) -> void {
	_radius = value;
}

auto PointLight::SetLocation(Vector3f value) -> void {
	_location = value;
}

}
