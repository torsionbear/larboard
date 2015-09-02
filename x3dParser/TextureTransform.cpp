#include "TextureTransform.h"

using std::string;
using std::stof;

namespace x3dParser {
auto TextureTransform::SetAttribute(string const& attribute, string&& value) -> void {
    if(attribute.compare("translation") == 0) {
        SetTranslation(move(value));
    } else if (attribute.compare("scale") == 0) {
        SetScale(move(value));
    } else if (attribute.compare("rotation") == 0) {
        SetRotation(move(value));
    }
}
    
auto TextureTransform::AddChild(X3dNode * child) -> void {
}

auto TextureTransform::GetTranslation() const -> Float2 {
    return _translation;
}

auto TextureTransform::GetScale() const -> Float2 {
    return _scale;
}

auto TextureTransform::GetRotation() const -> Float {
    return _rotation;
}

auto TextureTransform::SetTranslation(string&& s) -> void {
    _translation = Float2{move(s)};
}

auto TextureTransform::SetScale(string&& s) -> void {
    _scale = Float2{move(s)};
}

auto TextureTransform::SetRotation(string&& s) -> void {
    _rotation = stof(s);
}

}