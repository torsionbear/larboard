#include "CubeMap.h"

#include <algorithm>

#include "PngReader.h"
#include "MessageLogger.h"
#include "TextureUsage.h"

using std::make_unique;

namespace core {

void swap(CubeMap& first, CubeMap& second) {
    using std::swap;
	swap(first._filenames, second._filenames);
	swap(first._data, second._data);
	swap(first._width, second._width);
	swap(first._height, second._height);
	swap(first._texture, second._texture);
}

auto CubeMap::Load() -> void {
    // todo: load image file according to file extension. Currently only png is supported
    for (auto i = 0u; i < 6; ++i) {
        PngReader pngReader{ _filenames[i] };
        pngReader.ReadPng();
        _width = pngReader.Width();
        _height = pngReader.Height();
        _data[i] = pngReader.GetData();
    }
}

}