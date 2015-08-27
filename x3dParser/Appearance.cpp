#include "Appearance.h"

using std::unique_ptr;

namespace x3dParser {
    
auto Appearance::SetAttribute(const std::string&, std::string&&) -> void {
}
    
auto Appearance::AddChild(pNode child) -> void {
    
    if(typeid(*child) == typeid(ImageTexture)) {
        _imageTexture.reset(static_cast<ImageTexture*>(child.release()));
    } else if(typeid(*child) == typeid(TextureTransform)) {
        _textureTransform.reset(static_cast<TextureTransform*>(child.release()));
    } else if(typeid(*child) == typeid(Material)) {
        _material.reset(static_cast<Material*>(child.release()));
    }
}

auto Appearance::GetImageTexture() -> unique_ptr<ImageTexture>& {
    return _imageTexture;
}

auto Appearance::GetTextureTransform() -> unique_ptr<TextureTransform>& {
    return _textureTransform;
}

auto Appearance::GetMaterial() -> unique_ptr<Material>& {
    return _material;
}

}