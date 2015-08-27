#include "Viewpoint.h"

using std::string;

namespace x3dParser {

auto Viewpoint::SetAttribute(const string& attribute, string&& value) -> void {
    if(attribute.compare("centerOfRotation") == 0) {
        SetCenterOfRotation(move(value));
    } else if (attribute.compare("position") == 0) {
        SetPosition(move(value));
    } else if (attribute.compare("orientation") == 0) {
        SetOrientation(move(value));
    } else if (attribute.compare("fieldOfView") == 0) {
        SetFieldOfView(move(value));
    } else if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    }
}
    
auto Viewpoint::AddChild(pNode) -> void {
}

auto Viewpoint::GetCenterOfRotation() const -> Float3 {
    return _centerOfRotation;
}

auto Viewpoint::GetPosition() const -> Float3 {
    return _position;
}

auto Viewpoint::GetOrientation() const -> Float4 {
    return _orientation;
}

auto Viewpoint::GetFieldOfView() const -> Float {
    return _fieldOfView;
}

auto Viewpoint::SetCenterOfRotation(string&& s) -> void {
    _centerOfRotation = Float3{move(s)};
}

auto Viewpoint::SetPosition(string&& s) -> void {
    _position = Float3{move(s)};
}

auto Viewpoint::SetOrientation(string&& s) -> void {
    _orientation = Float4{move(s)};
}

auto Viewpoint::SetFieldOfView(string&& s) -> void {
    _fieldOfView = stof(move(s));
}

}