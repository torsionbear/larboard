#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <utility>

#include "Matrix.h"
#include "Resource.h"

using std::vector;

namespace core {

class Texture : public Resource {
public:
	friend class ResourceManager;

public:
	enum TextureType : int {
		DiffuseMap = 0,
		SpecularMap = 1,
		NormalMap = 2,
		ParallaxMap = 3,
        HeightMap = 4,
	};

public:
	explicit Texture(std::string const& filename, TextureType type = DiffuseMap)
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
private:
	std::string _filename;
	std::vector<Vector4f> _data;
	unsigned int _width;
	unsigned int _height;
	openglUint _texture;
	TextureType _type;
};

}