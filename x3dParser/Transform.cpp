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
    }
}
  
auto Transform::AddChild(pNode child) -> void {
    if(typeid(*child) == typeid(Transform)) {
        _transform.emplace_back(static_cast<Transform*>(child.release()));
    } else if(typeid(*child) == typeid(Group)) {
        _group.reset(static_cast<Group*>(child.release()));
    } else if(typeid(*child) == typeid(Viewpoint)) {
        _viewpoint.reset(static_cast<Viewpoint*>(child.release()));
	} else if (typeid(*child) == typeid(PointLight)) {
		_pointLight.reset(static_cast<PointLight*>(child.release()));
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

auto Transform::GetTransform() -> std::vector<std::unique_ptr<Transform>>& {
    return _transform;
}

auto Transform::GetGroup() -> unique_ptr<Group>& {
    return _group;
}
    
auto Transform::GetViewpoint() -> unique_ptr<Viewpoint>& {
    return _viewpoint;
}

auto Transform::GetPointLight() -> std::unique_ptr<PointLight>& {
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