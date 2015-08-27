#include "Material.h"

#include <string>

using std::string;
using std::stof;

namespace x3dParser {

auto Material::SetAttribute(string const& attribute, string&& value) -> void {
    if(attribute.compare("diffuseColor") == 0) {
        SetDiffuseColor(move(value));
    } else if(attribute.compare("specularColor") == 0) {
        SetSpecularColor(move(value));
    } else if(attribute.compare("emissiveColor") == 0) {
        SetEmissiveColor(move(value));
    } else if(attribute.compare("ambientIntensity") == 0) {
        SetAmbientIntensity(move(value));
    } else if(attribute.compare("shininess") == 0) {
        SetShininess(move(value));
    } else if(attribute.compare("transparency") == 0) {
        SetTransparency(move(value));
    } else if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    }
}
    
auto Material::AddChild(pNode) -> void {
}

auto Material::GetDiffuseColor() const -> Float3 {
    return _diffuseColor;
}

auto Material::GetSpecularColor() const -> Float3 {
    return _specularColor;
}

auto Material::GetEmissiveColor() const -> Float3 {
    return _emissiveColor;
}

auto Material::GetAmbientIntensity() const -> Float {
    return _ambientIntensity;
}

auto Material::GetShininess() const -> Float {
    return _shininess;
}

auto Material::GetTransparency() const -> Float {
    return _transparency;
}

auto Material::SetDiffuseColor(string&& s) -> void {
    _diffuseColor = Float3{move(s)};
}

auto Material::SetSpecularColor(string&& s) -> void {
    _specularColor = Float3{move(s)};
}

auto Material::SetEmissiveColor(string&& s) -> void {
    _emissiveColor = Float3{move(s)};
}

auto Material::SetAmbientIntensity(string&& s) -> void{
    _ambientIntensity = stof(move(s));
}

auto Material::SetShininess(string&& s) -> void {
    _shininess = stof(move(s));
}

auto Material::SetTransparency(string&& s) -> void {
    _transparency = stof(move(s));
}

}