#pragma once

#include "X3dNode.h"
#include "BasicType.h"
#include "Coordinate.h"
#include "Normal.h"
#include "TextureCoordinate.h"

namespace x3dParser {

class IndexedFaceSet : public X3dNode {
public:
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetSolid() const -> bool;
    auto GetCreaseAngle() const -> Float;
    auto GetNormalPerVertex() const -> bool;
    auto GetTexCoordIndex() -> std::vector<ULong3>&;
    auto GetCoordIndex() -> std::vector<ULong3>&;
    auto GetCoordinate() -> std::unique_ptr<Coordinate>&;
    auto GetNormal() -> std::unique_ptr<Normal>&;
    auto GetTextureCoordinate() -> std::unique_ptr<TextureCoordinate>&;

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
    std::unique_ptr<Coordinate> _coordinate = nullptr;
    std::unique_ptr<Normal> _normal = nullptr;
    std::unique_ptr<TextureCoordinate> _textureCoordinate = nullptr;
};

}