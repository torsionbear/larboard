#include "IndexedTriangleSet.h"

#include <assert.h>

#include <boost/algorithm/string.hpp>

using std::string;
using std::vector;
using std::stringstream;

namespace x3dParser {

auto IndexedTriangleSet::SetAttribute(string const & attribute, string && value) -> void {
	if (attribute.compare("solid") == 0) {
		SetSolid(move(value));
	} else if (attribute.compare("normalPerVertex") == 0) {
		SetNormalPerVertex(move(value));
	} else if (attribute.compare("index") == 0) {
		SetIndex(move(value));
	}
}

auto IndexedTriangleSet::AddChild(X3dNode * child) -> void {
	if (typeid(*child) == typeid(Coordinate)) {
		_coordinate = static_cast<Coordinate*>(child);
	} else if (typeid(*child) == typeid(Normal)) {
		_normal = static_cast<Normal*>(child);
	} else if (typeid(*child) == typeid(TextureCoordinate)) {
		_textureCoordinate = static_cast<TextureCoordinate*>(child);
	}
}

auto IndexedTriangleSet::GetSolid() const -> bool {
	return _solid;
}

auto IndexedTriangleSet::GetNormalPerVertex() const -> bool {
	return _normalPerVertex;
}

auto IndexedTriangleSet::GetIndex() const -> vector<unsigned int> const& {
	return _index;
}

auto IndexedTriangleSet::GetCoordinate() const -> Coordinate const* {
	return _coordinate;
}

auto IndexedTriangleSet::GetNormal() const ->  Normal const* {
	return _normal;
}

auto IndexedTriangleSet::GetTextureCoordinate() const -> TextureCoordinate const * {
	return _textureCoordinate;
}

auto IndexedTriangleSet::SetSolid(string&& s) -> void {
	_solid = s.compare("true") == 0;
}

auto IndexedTriangleSet::SetNormalPerVertex(string&& s) -> void {
	_normalPerVertex = s.compare("true") == 0;
}

auto IndexedTriangleSet::SetIndex(string&& s) -> void {
	_index = ReadIndex(move(s));
}

auto IndexedTriangleSet::ReadIndex(string&& s) -> vector<unsigned int> {
	auto ret = vector<unsigned int>{};
	
	auto tokens = vector<string>{};
	boost::split(tokens, s, boost::algorithm::is_any_of(" "), boost::algorithm::token_compress_on);
	for (auto const& token : tokens) {
		if (token.empty()) {
			continue;
		}
		ret.push_back(std::stoi(token));
	}
	return ret;
}

}
