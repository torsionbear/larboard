#include "TextureArray.h"

#include "PngReader.h"

namespace core {
TextureArray::~TextureArray() {
}

auto TextureArray::Load() -> void {
    for (auto & filename : _filenames) {
        PngReader pngReader{ filename };
        pngReader.ReadPng();
        _width = pngReader.Width();
        _height = pngReader.Height();
        _data.push_back(pngReader.GetData());
    }
}

}
