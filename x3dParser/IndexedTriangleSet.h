#pragma once

#include "X3dNode.h"
#include "BasicType.h"
#include "Coordinate.h"
#include "Normal.h"
#include "TextureCoordinate.h"

namespace x3dParser {

class IndexedTriangleSet : public X3dNode {
public:
	auto SetAttribute(std::string const&, std::string&&) -> void override;
	auto AddChild(X3dNode *) -> void override;

	auto GetSolid() const -> bool;
	auto GetNormalPerVertex() const -> bool;
	auto GetIndex() const->std::vector<unsigned int> const&;
	auto GetCoordinate() const->Coordinate const*;
	auto GetNormal() const->Normal const*;
	auto GetTextureCoordinate() const->TextureCoordinate const*;

private:
	auto SetSolid(std::string&&) -> void;
	auto SetNormalPerVertex(std::string&&) -> void;
	auto SetIndex(std::string&&) -> void;

	auto ReadIndex(std::string&&)->std::vector<unsigned int>;

private:
	bool _solid;
	bool _normalPerVertex;
	std::vector<unsigned int> _index;
	Coordinate * _coordinate = nullptr;
	Normal * _normal = nullptr;
	TextureCoordinate * _textureCoordinate = nullptr;
};

}