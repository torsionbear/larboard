#pragma once

#include "X3dNode.h"

#include "BasicType.h"

namespace x3dParser {

class TextureTransform : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode * child) -> void override;

    auto GetTranslation() const -> Float2;
    auto GetScale() const -> Float2;
    auto GetRotation() const -> Float;

private:
    auto SetTranslation(std::string&&) -> void;
    auto SetScale(std::string&&) -> void;
    auto SetRotation(std::string&&) -> void;

private:
    Float2 _translation;
    Float2 _scale;
    Float _rotation;
};

}