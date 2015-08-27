#include "BasicType.h"

using std::string;
using std::stof;

namespace x3dParser {
    
    
Float2::Float2(Float x, Float y) 
    : x(x), y(y) {
}

Float2::Float2(string&& s) {
    auto pos = string::size_type{};
    x = stof(s, &pos);
    y = stof(s.substr(pos));
}

auto Float2::operator==(Float2 const& rhs) const -> bool {
    return equal(x, rhs.x) && equal(y, rhs.y);
}
    
Float3::Float3(Float x, Float y, Float z) 
    : x(x), y(y), z(z) {
}

Float3::Float3(string&& s) {
    auto pos = string::size_type{};
    x = stof(s, &pos);
    y = stof(s = s.substr(pos), &pos);
    z = stof(s.substr(pos));
}

auto Float3::operator==(Float3 const& rhs) const -> bool {
    return equal(x, rhs.x) && equal(y, rhs.y) && equal(z, rhs.z);
}
    
Float4::Float4(Float x, Float y, Float z, Float a) 
    : x(x), y(y), z(z) , a(a) {
}

Float4::Float4(string&& s) {
    auto pos = string::size_type{};
    x = stof(s, &pos);
    y = stof(s = s.substr(pos), &pos);
    z = stof(s = s.substr(pos), &pos);
    a = stof(s.substr(pos));
}

auto Float4::operator==(Float4 const& rhs) const -> bool {
    return equal(x, rhs.x) && equal(y, rhs.y) && equal(z, rhs.z) & equal(a, rhs.a);
}

ULong3::ULong3(ULong a, ULong b, ULong c) 
    : a(a), b(b), c(c) {
}

auto ULong3::operator==(ULong3 const& rhs) const -> bool {
    return equal(a, rhs.a) && equal(b, rhs.b) && equal(c, rhs.c);
}

}