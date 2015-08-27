#include "IndexedFaceSet.h"

#include <string>
#include <sstream>
#include <assert.h>

using std::string;
using std::stof;
using std::stringstream;
using std::vector;
using std::unique_ptr;
using std::move;

namespace x3dParser {

auto IndexedFaceSet::SetAttribute(string const& attribute, string&& value) -> void {
    if(attribute.compare("solid") == 0) {
        SetSolid(move(value));
    } else if(attribute.compare("creaseAngle") == 0) {
        SetCreaseAngle(move(value));
    } else if(attribute.compare("normalPerVertex") == 0) {
        SetNormalPerVertex(move(value));
    } else if(attribute.compare("texCoordIndex") == 0) {
        SetTexCoordIndex(move(value));
    } else if(attribute.compare("coordIndex") == 0) {
        SetCoordIndex(move(value));
    }
}

auto IndexedFaceSet::AddChild(pNode child) -> void {
    if(typeid(*child) == typeid(Coordinate)) {
        _coordinate.reset(static_cast<Coordinate*>(child.release()));
    } else if (typeid(*child) == typeid(Normal)) {
        _normal.reset(static_cast<Normal*>(child.release()));
    } else if (typeid(*child) == typeid(TextureCoordinate)) {
        _textureCoordinate.reset(static_cast<TextureCoordinate*>(child.release()));
    }
}

auto IndexedFaceSet::GetSolid() const -> bool {
    return _solid;
}

auto IndexedFaceSet::GetCreaseAngle() const -> Float {
    return _creaseAngle;
}

auto IndexedFaceSet::GetNormalPerVertex() const -> bool {
    return _normalPerVertex;
}

auto IndexedFaceSet::GetTexCoordIndex() -> vector<ULong3>& {
    return _texCoordIndex;
}

auto IndexedFaceSet::GetCoordIndex() -> vector<ULong3>& {
    return _coordIndex;
}
    
auto IndexedFaceSet::GetCoordinate() -> unique_ptr<Coordinate>& {
    return _coordinate;
}

auto IndexedFaceSet::GetNormal() -> unique_ptr<Normal>& {
    return _normal;
}

auto IndexedFaceSet::GetTextureCoordinate() -> unique_ptr<TextureCoordinate>& {
    return _textureCoordinate;
}

auto IndexedFaceSet::SetSolid(string&& s) -> void {
    _solid = s.compare("true") == 0;
}

auto IndexedFaceSet::SetCreaseAngle(string&& s) -> void {
    _creaseAngle = stof(move(s));
}

auto IndexedFaceSet::SetNormalPerVertex(string&& s) -> void {
    _normalPerVertex = s.compare("true") == 0;
}

auto IndexedFaceSet::SetTexCoordIndex(string&& s) -> void {
    _texCoordIndex = ReadIndex(move(s));
}

auto IndexedFaceSet::SetCoordIndex(string&& s) -> void {
    _coordIndex = ReadIndex(move(s));
}

auto IndexedFaceSet::ReadIndex(string&& s) -> vector<ULong3> {
    auto ret = vector<ULong3>{};
    auto ss = stringstream(move(s));
    auto index = ULong3{};
    while(ss >> index.a >> index.b >> index.c) {
        auto separator = 0;
        ss >> separator;
        assert(separator == -1);
        ret.emplace_back(index);
    }
    return ret;
}
}