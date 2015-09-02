#include "Normal.h"

#include <sstream>

using std::string;
using std::stringstream;
using std::move;
using std::vector;

namespace x3dParser {

auto Normal::SetAttribute(string const& attribute, string&& value) -> void {
    if(attribute.compare("vector") == 0) {
        SetVector(move(value));
    } else if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    } else if (attribute.compare("USE") == 0) {
		SetUse(move(value));
	}
}
    
auto Normal::AddChild(X3dNode *) -> void {
}

auto Normal::GetVector() const -> vector<Float3> const& {
    return _vector;
}

auto Normal::SetVector(string&& s) -> void {
    auto ss = stringstream{move(s)};

    auto point = Float3{};
    while(ss >> point.x >> point.y >> point.z) {
        _vector.emplace_back(point);
    }
}

}