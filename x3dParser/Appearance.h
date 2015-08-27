#pragma once

#include "X3dNode.h"
#include "ImageTexture.h"
#include "TextureTransform.h"
#include "Material.h"

namespace x3dParser {

class Appearance : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;

    auto GetImageTexture() -> std::unique_ptr<ImageTexture>&;
    auto GetTextureTransform() -> std::unique_ptr<TextureTransform>&;
    auto GetMaterial() -> std::unique_ptr<Material>&;

private:
    std::unique_ptr<ImageTexture> _imageTexture = nullptr;
    std::unique_ptr<TextureTransform> _textureTransform = nullptr;
    std::unique_ptr<Material> _material = nullptr;
};

}