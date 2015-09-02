#pragma once

#include "X3dNode.h"
#include "ImageTexture.h"
#include "TextureTransform.h"
#include "Material.h"

namespace x3dParser {

class Appearance : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode * child) -> void override;

    auto GetImageTexture() const -> ImageTexture const*;
    auto GetTextureTransform() const -> TextureTransform const*;
    auto GetMaterial() const -> Material const*;

private:
    ImageTexture * _imageTexture = nullptr;
    TextureTransform  * _textureTransform = nullptr;
    Material * _material = nullptr;
};

}