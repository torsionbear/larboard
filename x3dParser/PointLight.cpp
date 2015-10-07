#include "PointLight.h"

using std::string;

namespace x3dParser {

auto PointLight::SetAttribute(string const& attribute, string&& value) -> void {
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
	} else if (attribute.compare("DEF") == 0) {
		SetDef(move(value));
	} else if (attribute.compare("USE") == 0) {
		SetUse(move(value));
	}
}

auto PointLight::AddChild(X3dNode *) -> void {
	return;
}

auto PointLight::GetAmbientIntensity() const -> Float {
	return _ambientIntensity;
}

auto PointLight::GetColor() const -> Float3 {
	return _color;
}

auto PointLight::GetIntensity() const -> Float {
	return _intensity;
}

auto PointLight::GetRadius() const -> Float {
	return _radius;
}

auto PointLight::GetLocation() const -> Float3 {
	return _location;
}

auto PointLight::GetAttenuation() const -> Float3 {
	return _attenuation;
}

auto PointLight::SetAmbientIntensity(string && s) -> void {
	_ambientIntensity = stof(move(s));
}

auto PointLight::SetColor(string && s) -> void {
	_color = Float3{move(s)};
}

auto PointLight::SetIntensity(string && s) -> void {
	_intensity = stof(move(s));
}

auto PointLight::SetRadius(string && s) -> void {
	_radius = stof(move(s));
}

auto PointLight::SetLocation(string && s) -> void {
	_location = Float3{ move(s) };
}

auto PointLight::SetAttenuation(std::string && s) -> void {
	_attenuation = Float3{ move(s) };
}

}
