#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <utility>

#include "Matrix.h"
#include "Resource.h"

using std::vector;

namespace core {

class CubeMap : public Resource {
public:
	explicit CubeMap(std::array<std::string, 6> && filenames)  // right, left, front, back, top, bottom
		: _filenames(filenames)
		, _texture(0) {
	}
    CubeMap(CubeMap const&) = delete;
    CubeMap(CubeMap && other) {
		swap(*this, other);
	}
    CubeMap& operator=(CubeMap const&) = delete;
    CubeMap& operator=(CubeMap && other) {
		swap(*this, other);
		return *this;
	}
	~CubeMap() = default;
	friend void swap(CubeMap& lhs, CubeMap& rhs);

private:
	auto LoadImpl() -> bool override;
	auto UnloadImpl() -> bool override;
	auto SendToCardImpl() -> bool override;
	auto FreeFromCardImpl() -> bool override;

public:
	auto Use() -> void;
private:
	std::array<std::string, 6> _filenames;
    std::array<std::vector<Vector4f>, 6> _data;
	unsigned int _width;
	unsigned int _height;
	openglUint _texture;
};

}