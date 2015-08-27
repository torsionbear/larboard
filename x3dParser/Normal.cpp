#include "Normal.h"

#include <sstream>

using std::string;
using std::stringstream;
using std::move;
using std::vector;

namespace x3dParser {

auto Normal::SetAttribute(const string& attribute, string&& value) -> void {
    if(attribute.compare("vector") == 0) {
        SetVector(move(value));
    } else if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    }
}
    
auto Normal::AddChild(pNode) -> void {
}

auto Normal::GetVector() -> vector<Float3>& {
    return _vector;
}

auto Normal::StealVector() -> vector<Float3> {
    auto ret = vector<Float3>{};
    ret.swap(_vector);
    return ret;
}

auto Normal::SetVector(string&& s) -> void {
    auto ss = stringstream{move(s)};

    auto point = Float3{};
    while(ss >> point.x >> point.y >> point.z) {
        _vector.emplace_back(point);
    }
}

}