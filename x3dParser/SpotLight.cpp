#include "SpotLight.h"

using std::string;

namespace x3dParser {

auto SpotLight::SetAttribute(string const& attribute, string&& value) -> void {
	if (attribute.compare("ambientIntensity") == 0) {
		SetAmbientIntensity(move(value));
	} else if (attribute.compare("color") == 0) {
		SetColor(move(value));
	} else if (attribute.compare("intensity") == 0) {
		SetIntensity(move(value));
	} else if (attribute.compare("radius") == 0) {
		SetRadius(move(value));
	} else if (attribute.compare("location") == 0) {
		SetLocation(move(value));
	} else if (attribute.compare("attenuation") == 0) {
		SetAttenuation(move(value));
	} else if (attribute.compare("direction") == 0) {
		SetDirection(move(value));
	} else if (attribute.compare("beamWidth") == 0) {
		SetBeamWidth(move(value));
	} else if (attribute.compare("cutOffAngle") == 0) {
		SetCutOffAngle(move(value));
	} else if (attribute.compare("DEF") == 0) {
		SetDef(move(value));
	} else if (attribute.compare("USE") == 0) {
		SetUse(move(value));
	}
}

auto SpotLight::AddChild(X3dNode *) -> void {
	return;
}

auto SpotLight::GetAmbientIntensity() const -> Float {
	return _ambientIntensity;
}

auto SpotLight::GetColor() const -> Float3 {
	return _color;
}

auto SpotLight::GetIntensity() const -> Float {
	return _intensity;
}

auto SpotLight::GetRadius() const -> Float {
	return _radius;
}

auto SpotLight::GetLocation() const -> Float3 {
	return _location;
}

auto SpotLight::GetAttenuation() const -> Float3 {
	return _attenuation;
}

auto SpotLight::GetDirection() const -> Float3 {
	return _direction;
}

auto SpotLight::GetBeamWidth() const -> Float {
	return _beamWidth;
}

auto SpotLight::GetCutOffAngle() const -> Float {
	return _cutOffAngle;
}

auto SpotLight::SetAmbientIntensity(string && s) -> void {
	_ambientIntensity = stof(move(s));
}

auto SpotLight::SetColor(string && s) -> void {
	_color = Float3{ move(s) };
}

auto SpotLight::SetIntensity(string && s) -> void {
	_intensity = stof(move(s));
}

auto SpotLight::SetRadius(string && s) -> void {
	_radius = stof(move(s));
}

auto SpotLight::SetLocation(string && s) -> void {
	_location = Float3{ move(s) };
}

auto SpotLight::SetAttenuation(std::string && s) -> void {
	_attenuation = Float3{ move(s) };
}

auto SpotLight::SetDirection(std::string && s) -> void {
	_direction = Float3{ move(s) };
}

auto SpotLight::SetBeamWidth(std::string && s) -> void {
	_beamWidth = stof(move(s));
}

auto SpotLight::SetCutOffAngle(std::string && s) -> void {
	_cutOffAngle = stof(move(s));
}

}