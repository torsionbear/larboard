#include "Texture.h"

#include <algorithm>

#include <GL/glew.h>

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

auto core::Texture::LoadImpl() -> bool {
	// only support png yet
	PngReader pngReader{_filename};
	pngReader.ReadPng();
	_width = pngReader.Width();
	_height = pngReader.Height();
	_data = pngReader.GetData();
	return true;
}

auto Texture::UnloadImpl() -> bool {
	_data.clear();
	return true;
}

auto Texture::SendToCardImpl() -> bool {
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	auto levelCount = static_cast<GLsizei>(floor(log2(std::max(_width, _height))) + 1);
	glTexStorage2D(GL_TEXTURE_2D, levelCount, GL_RGBA32F, _width, _height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_FLOAT, _data.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// todo: wrap glGetError(), in release code, calling it only once per frame untill error happpens
	auto error = glGetError();
	if (GL_NO_ERROR != error) {
		exit(EXIT_FAILURE);
		return false;
	}
	return true;
}

auto Texture::FreeFromCardImpl() -> bool {
	glDeleteTextures(1, &_texture);
	return true;
}

auto Texture::Use(unsigned int index) -> void {
	assert(status::SentToCard == _status);

	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, _texture);

}

}