#include "CubeMap.h"

#include <algorithm>

#include <GL/glew.h>

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

auto CubeMap::LoadImpl() -> bool {
	// todo: load image file according to file extension. Currently only png is supported
    for (auto i = 0u; i < 6; ++i) {
        PngReader pngReader{ _filenames[i] };
        pngReader.ReadPng();
        _width = pngReader.Width();
        _height = pngReader.Height();
        _data[i] = pngReader.GetData();
    }
	return true;
}

auto CubeMap::UnloadImpl() -> bool {
    for (auto & data : _data) {
        data.clear();
    }
	return true;
}

auto CubeMap::SendToCardImpl() -> bool {
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
	auto levelCount = static_cast<GLsizei>(floor(log2(std::max(_width, _height))) + 1);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, levelCount, GL_RGBA32F, _width, _height);
    for (auto i = 0u; i < 6; ++i) {
        auto target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
        glTexSubImage2D(target, 0, 0, 0, _width, _height, GL_RGBA, GL_FLOAT, _data[i].data());
    }
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	// todo: wrap glGetError(), in release code, calling it only once per frame untill error happpens
	auto error = glGetError();
	if (GL_NO_ERROR != error) {
        MessageLogger::Log(MessageLogger::Error, "generating cube map failed.");
		return false;
	}
	return true;
}

auto CubeMap::FreeFromCardImpl() -> bool {
	glDeleteTextures(1, &_texture);
	return true;
}

auto CubeMap::Use() -> void {
	assert(status::SentToCard == _status);

	glActiveTexture(GL_TEXTURE0 + TextureUsage::CubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);

}

}