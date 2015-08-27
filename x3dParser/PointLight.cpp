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
	} else if (attribute.compare("DEF") == 0) {
		SetDef(move(value));
	}
}

auto PointLight::AddChild(pNode) -> void {
	return;
}

auto PointLight::GetAmbientIntensity() -> Float {
	return _ambientIntensity;
}

auto PointLight::GetColor() -> Float3 {
	return _color;
}

auto PointLight::GetIntensity() -> Float {
	return _intensity;
}

auto PointLight::GetRadius() -> Float {
	return _radius;
}

auto PointLight::GetLocation() -> Float3 {
	return _location;
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

}
