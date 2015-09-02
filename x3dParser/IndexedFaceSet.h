#pragma once

#include "X3dNode.h"
#include "BasicType.h"
#include "Coordinate.h"
#include "Normal.h"
#include "TextureCoordinate.h"

namespace x3dParser {

class IndexedFaceSet : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode *) -> void override;

    auto GetSolid() const -> bool;
    auto GetCreaseAngle() const -> Float;
    auto GetNormalPerVertex() const -> bool;
    auto GetTexCoordIndex() const -> std::vector<ULong3> const&;
    auto GetCoordIndex() const -> std::vector<ULong3> const&;
    auto GetCoordinate() const -> Coordinate const*;
    auto GetNormal() const -> Normal const*;
    auto GetTextureCoordinate() const -> TextureCoordinate const*;

private:
    auto SetSolid(std::string&&) -> void;
    auto SetCreaseAngle(std::string&&) -> void;
    auto SetNormalPerVertex(std::string&&) -> void;
    auto SetTexCoordIndex(std::string&&) -> void;
    auto SetCoordIndex(std::string&&) -> void;

    auto ReadIndex(std::string&&) -> std::vector<ULong3>;

private:
    bool _solid;
    Float _creaseAngle;
    bool _normalPerVertex;
    std::vector<ULong3> _texCoordIndex;
    std::vector<ULong3> _coordIndex;
    Coordinate * _coordinate = nullptr;
    Normal * _normal = nullptr;
    TextureCoordinate * _textureCoordinate = nullptr;
};

}