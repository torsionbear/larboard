#include "Group.h"

using std::string;
using std::vector;

namespace x3dParser {
    
auto Group::SetAttribute(const std::string& attribute, std::string&& value) -> void {
    if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    }
}

auto Group::AddChild(pNode child) -> void {
    if(typeid(*child) == typeid(Shape)) {
        _shape.emplace_back(static_cast<Shape*>(child.release()));
    }
}

auto Group::GetShape() -> std::vector<std::unique_ptr<Shape>>& {
    return _shape;
}

}