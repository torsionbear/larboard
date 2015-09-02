#include "Appearance.h"

using std::unique_ptr;

namespace x3dParser {
    
auto Appearance::SetAttribute(std::string const&, std::string&&) -> void {
}
    
auto Appearance::AddChild(X3dNode * child) -> void {
    
    if(typeid(*child) == typeid(ImageTexture)) {
        _imageTexture = static_cast<ImageTexture*>(child);
    } else if(typeid(*child) == typeid(TextureTransform)) {
        _textureTransform = static_cast<TextureTransform*>(child);
    } else if(typeid(*child) == typeid(Material)) {
        _material = static_cast<Material*>(child);
    }
}

auto Appearance::GetImageTexture() const -> ImageTexture const* {
    return _imageTexture;
}

auto Appearance::GetTextureTransform() const -> TextureTransform const* {
    return _textureTransform;
}

auto Appearance::GetMaterial() const -> Material const* {
    return _material;
}

}