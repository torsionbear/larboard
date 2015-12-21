#include "Texture.h"

#include <algorithm>

#include "PngReader.h"
#include "MessageLogger.h"

using std::make_unique;

namespace core {

void swap(Texture& first, Texture& second) {
	std::swap(first._filename, second._filename);
	std::swap(first._data, second._data);
	std::swap(first._width, second._width);
	std::swap(first._height, second._height);
	std::swap(first._texture, second._texture);
}

auto Texture::Load() -> void {
    // todo: load image file according to file extension. Currently only png is supported
    PngReader pngReader{ _filename };
    pngReader.ReadPng();
    _width = pngReader.Width();
    _height = pngReader.Height();
    _data = pngReader.GetData();
}

auto Texture::GetTexel(int x, int y) const -> Vector4f {
    x = x % _width;
    y = y % _height;
    return _data[y * _width + x];
}

auto Texture::GetBilinearFilteredTexel(Float32 coord0, Float32 coord1) const -> Vector4f {
    auto coordX = coord0 * _width - 0.5f;
    auto coordY = coord1 * _height - 0.5f;

    auto indexX = static_cast<int>(floor(coordX));
    auto indexY = static_cast<int>(floor(coordY));

    auto texel00 = GetTexel(indexX, indexY );
    auto texel10 = GetTexel(indexX + 1, indexY);
    auto texel01 = GetTexel(indexX, indexY + 1);
    auto texel11 = GetTexel(indexX + 1, indexY + 1);

    auto x_ratio = coordX - indexX;
    auto y_ratio = coordY - indexY;
    auto x_opposite = 1 - x_ratio;
    auto y_opposite = 1 - y_ratio;

    // the following 2 lines will cause crash in release version!
    // temporary (texel00 * x_opposite + texel10 * x_ratio) is destroyed after first line, in second line, result's reference to it will be dangling.
    // (see "effective modern c++" item 6)
    //auto result = (texel00 * x_opposite + texel10 * x_ratio) * y_opposite + (texel01 * x_opposite + texel11 * x_ratio) * y_ratio;
    //return result;
    return (texel00 * x_opposite + texel10 * x_ratio) * y_opposite + (texel01 * x_opposite + texel11 * x_ratio) * y_ratio;
}

}