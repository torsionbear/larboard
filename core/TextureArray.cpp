#include "TextureArray.h"

#include <GL/glew.h>

#include "PngReader.h"

namespace core {
TextureArray::~TextureArray() {
}
auto TextureArray::Load() -> void {
    LoadFile();

    auto error = glGetError();
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
    auto levelCount = static_cast<GLsizei>(floor(log2(std::max(_width, _height))) + 1);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, levelCount, GL_RGBA32F, _width, _height, _data.size());
    for (auto i = 0u; i < _data.size(); ++i) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, _width, _height, 1, GL_RGBA, GL_FLOAT, _data[i].data());
        error = glGetError();
    }
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // repeat texture(suppose it's seamless)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    error = glGetError();
    assert(GL_NO_ERROR == error);
}

auto TextureArray::Use() -> void {
    glActiveTexture(GL_TEXTURE0 + _type);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
}

auto TextureArray::LoadFile() -> void {
    for (auto & filename : _filenames) {
        PngReader pngReader{ filename };
        pngReader.ReadPng();
        _width = pngReader.Width();
        _height = pngReader.Height();
        _data.push_back(pngReader.GetData());
    }
}

}
