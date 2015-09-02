#include "TextureCoordinate.h"

#include <sstream>

using std::string;
using std::stringstream;
using std::move;
using std::vector;

namespace x3dParser {

auto TextureCoordinate::SetAttribute(string const& attribute, string&& value) -> void {
    if(attribute.compare("point") == 0) {
        SetPoint(move(value));
    }
}
    
auto TextureCoordinate::AddChild(X3dNode * child) -> void {
}

auto TextureCoordinate::GetPoint() const -> const vector<Float2>& {
    return _point;
}

auto TextureCoordinate::StealPoint() -> vector<Float2> {
    auto ret = vector<Float2>{};
    ret.swap(_point);
    return ret;
}

auto TextureCoordinate::SetPoint(string&& s) -> void {
    auto ss = stringstream{move(s)};

    auto point = Float2{};
    while(ss >> point.x >> point.y) {
        _point.push_back(point);
    }
}

}