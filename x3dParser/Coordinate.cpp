#include "Coordinate.h"

#include <sstream>

using std::string;
using std::stringstream;
using std::vector;

namespace x3dParser {

auto Coordinate::SetAttribute(string const& attribute, string&& value) -> void {
    if(attribute.compare("point") == 0) {
        SetPoint(move(value));
    } else if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    } else if (attribute.compare("USE") == 0) {
		SetUse(move(value));
	}
}
    
auto Coordinate::AddChild(X3dNode *) -> void {
}

auto Coordinate::GetPoint() const -> vector<Float3> const& {
    return _point;
}

auto Coordinate::SetPoint(string&& s) -> void{
    auto ss = stringstream{move(s)};

    auto point = Float3{};
    while(ss >> point.x >> point.y >> point.z) {
        _point.push_back(point);
    }
}

}