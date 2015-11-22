#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <utility>

#include "Matrix.h"
#include "Resource.h"
#include "TextureUsage.h"

using std::vector;

namespace core {

class Texture : public Resource {
public:
	friend class ResourceManager;
public:
	explicit Texture(std::string const& filename, TextureUsage::TextureType type = TextureUsage::DiffuseMap)
		: _filename(filename)
		, _texture(0)
		, _type(type) {
	}
	Texture(Texture const&) = delete;
	Texture(Texture && other) {
		swap(*this, other);
	}
	Texture& operator=(Texture const&) = delete;
	Texture& operator=(Texture && other) {
		swap(*this, other);
		return *this;
	}
	~Texture() = default;
	friend void swap(Texture& lhs, Texture& rhs);

private:
	auto LoadImpl() -> bool override;
	auto UnloadImpl() -> bool override;
	auto SendToCardImpl() -> bool override;
	auto FreeFromCardImpl() -> bool override;

public:
	auto Use() -> void;
    auto GetBilinearFilteredTexel(Float32 x, Float32 y) const -> Vector4f;
    auto GetWidth() const -> unsigned int {
        return _width;
    }
    auto GetHeight() const -> unsigned int {
        return _height;
    }
private:
    auto GetTexel(int x, int y) const->Vector4f;
private:
	std::string _filename;
	std::vector<Vector4f> _data;
	unsigned int _width;
	unsigned int _height;
	openglUint _texture;
    TextureUsage::TextureType _type;
};

}