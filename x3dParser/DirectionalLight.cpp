#include "DirectionalLight.h"

using std::string;

namespace x3dParser {

auto DirectionalLight::SetAttribute(string const& attribute, string && value) -> void {
	if (attribute.compare("ambientIntensity") == 0) {
		SetAmbientIntensity(move(value));
	} else if (attribute.compare("color") == 0) {
		SetColor(move(value));
	} else if (attribute.compare("intensity") == 0) {
		SetIntensity(move(value));
	} else if (attribute.compare("direction") == 0) {
		SetDirection(move(value));
	} else if (attribute.compare("DEF") == 0) {
		SetDef(move(value));
	} else if (attribute.compare("USE") == 0) {
		SetUse(move(value));
	}

}

auto DirectionalLight::AddChild(X3dNode *) -> void {
	return;
}

auto DirectionalLight::GetAmbientIntensity() const -> Float {
	return _ambientIntensity;
}

auto DirectionalLight::GetColor() const -> Float3 {
	return _color;
}

auto DirectionalLight::GetIntensity() const -> Float {
	return _intensity;
}

auto DirectionalLight::GetDirection() const -> Float3 {
	return _direction;
}

auto DirectionalLight::SetAmbientIntensity(std::string && s) -> void {
	_ambientIntensity = stof(move(s));
}

auto DirectionalLight::SetColor(std::string && s) -> void {
	_color = Float3{ move(s) };
}

auto DirectionalLight::SetIntensity(std::string && s) -> void {
	_intensity = stof(move(s));
}

auto DirectionalLight::SetDirection(std::string && s) -> void {
	_direction = Float3{ move(s) };
}

}