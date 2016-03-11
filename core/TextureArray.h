#pragma once

#include <string>
#include <vector>

#include "Matrix.h"
#include "TextureUsage.h"

namespace core {

class TextureArray {
public:
    TextureArray(std::vector<std::string> && filenames, TextureUsage::TextureType type = TextureUsage::DiffuseTextureArray)
        : _filenames(move(filenames))
        , _type(type) {
    }
    ~TextureArray();
public:
    auto Load() -> void;

    auto GetWidth() const -> unsigned int {
        return _width;
    }
    auto GetHeight() const -> unsigned int {
        return _height;
    }
    auto SetTexture(openglUint texture) {
        _texture = texture;
    }
    auto GetTexture() const -> openglUint {
        return _texture;
    }
    auto GetData() -> std::vector<std::vector<Vector4f>> const& {
        return _data;
    }
    auto GetFilenames() const -> std::vector<std::string> const& {
        return _filenames;
    }
private:
    std::vector<std::string> _filenames;
    std::vector<std::vector<Vector4f>> _data;
    unsigned int _width;
    unsigned int _height;
    openglUint _texture;
    TextureUsage::TextureType _type;
public:
    unsigned int _renderDataId;
};

}