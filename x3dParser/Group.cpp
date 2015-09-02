#include "Group.h"

using std::string;
using std::vector;

namespace x3dParser {
    
auto Group::SetAttribute(std::string const& attribute, std::string&& value) -> void {
    if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    } else if (attribute.compare("USE") == 0) {
		SetUse(move(value));
	}
}

auto Group::AddChild(X3dNode * child) -> void {
    if(typeid(*child) == typeid(Shape)) {
        _shape.push_back(static_cast<Shape*>(child));
    }
}

auto Group::GetShape() const -> std::vector<Shape *> const& {
    return _shape;
}

}