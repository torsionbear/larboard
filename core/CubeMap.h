#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <utility>

#include "Matrix.h"
#include "Resource.h"

using std::vector;

namespace core {

class CubeMap {
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
public:
    auto Load() -> void;
    auto GetTexture() const -> openglUint {
        return _texture;
    }
    auto SetTexture(openglUint texture) -> void {
        _texture = texture;
    }
    auto GetWidth() const {
        return _width;
    }
    auto GetHeight() const {
        return _height;
    }
    auto GetData() const->std::array<std::vector<Vector4f>, 6> const& {
        return _data;
    }
private:
    std::array<std::string, 6> _filenames;
    std::array<std::vector<Vector4f>, 6> _data;
    unsigned int _width;
    unsigned int _height;
    openglUint _texture;
};

}