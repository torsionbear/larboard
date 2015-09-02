#include "Transform.h"

using std::string;
using std::unique_ptr;

namespace x3dParser {

auto Transform::SetAttribute(string const& attribute, string&& value) -> void {
    if(attribute.compare("translation") == 0) {
        SetTranslation(move(value));
    } else if (attribute.compare("scale") == 0) {
        SetScale(move(value));
    } else if (attribute.compare("rotation") == 0) {
        SetRotation(move(value));
    } else if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    } else if (attribute.compare("USE") == 0) {
		SetUse(move(value));
	}
}
  
auto Transform::AddChild(X3dNode * child) -> void {
    if(typeid(*child) == typeid(Transform)) {
        _transform.push_back(static_cast<Transform*>(child));
    } else if(typeid(*child) == typeid(Group)) {
        _group = static_cast<Group*>(child);
    } else if(typeid(*child) == typeid(Viewpoint)) {
        _viewpoint = static_cast<Viewpoint*>(child);
	} else if (typeid(*child) == typeid(PointLight)) {
		_pointLight = static_cast<PointLight*>(child);
	}
}

auto Transform::GetTranslation() const -> Float3 {
    return _translation;
}

auto Transform::GetScale() const -> Float3 {
    return _scale;
}

auto Transform::GetRotation() const -> Float4 {
    return _rotation;
}

auto Transform::GetTransform() const -> std::vector<Transform *> const& {
    return _transform;
}

auto Transform::GetGroup() const -> Group const* {
    return _group;
}
    
auto Transform::GetViewpoint() const -> Viewpoint const* {
    return _viewpoint;
}

auto Transform::GetPointLight() const -> PointLight const* {
	return _pointLight;
}

auto Transform::SetTranslation(string&& s) -> void {
    _translation = Float3{move(s)};
}

auto Transform::SetScale(string&& s) -> void {
    _scale = Float3{move(s)};
}

auto Transform::SetRotation(string&& s) -> void {
    _rotation = Float4{move(s)};
}

}