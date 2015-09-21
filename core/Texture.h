#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <utility>

#include "Vector.h"
#include "Resource.h"

using std::vector;

namespace core {

class Texture : public Resource {
public:
	friend class ResourceManager;

public:
	explicit Texture(std::string const& filename)
		: _filename(filename)
		, _texture(0) {
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
	auto Use(unsigned int) -> void;

private:
	std::string _filename;
	std::vector<Vector4f> _data;
	unsigned int _width;
	unsigned int _height;
	openglUint _texture;
};

}