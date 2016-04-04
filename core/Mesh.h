#pragma once

#include <vector>
#include <memory>

#include "Vertex.h"
#include "Resource.h"

namespace core {

template <typename T>
class Mesh {
public:
    enum DrawMode {
        triangles,
        patches,
    };
    struct RenderData {
        openglUint _vao;
        openglUint _indexOffset;
        openglInt _baseVertex;
    };
    static auto MakeMesh(Float32 * vertexData, unsigned int vertexDataCount, unsigned int * index, unsigned int indexCount) -> void {
        auto ret = Mesh{};
        ret._index = std::vector<unsigned int>()
    }
public:
    friend void swap(Mesh& first, Mesh& second) {
        using std::swap;
        swap(first._vertexes, second._vertexes);
        swap(first._renderData, second._renderData);
    }
    Mesh(DrawMode drawMode = triangles)
        : _drawMode(drawMode) {
    }
    Mesh(std::vector<T> && vertexData, std::vector<unsigned int> && index)
        : _vertexes(move(vertexData))
        , _index(move(index)) {
    }
    Mesh(std::vector<Float32> const& vertexData, std::vector<unsigned int> && index)
        : _index(move(index)) {
        auto vertexCount = vertexData.size() * sizeof(Float32) / sizeof(T);
        _vertexes = std::vector<T>(vertexCount);
        memcpy(_vertexes.data(), vertexData.data(), vertexData.size() * sizeof(Float32));
    }
    Mesh(std::vector<T> && vertexData)
        : _vertexes(move(vertexData)) {
        _index.resize(_vertexes.size());
        for (auto i = 0u; i < _vertexes.size(); ++i) {
            _index[i] = i;
        }
    }
    Mesh(Mesh&& other)
        : Mesh() {
        swap(*this, other);
    }
    ~Mesh() = default;
    Mesh& operator=(Mesh rhs) {
        swap(*this, rhs);
        return *this;
    }
    Mesh& operator=(Mesh && rhs) {
        swap(*this, rhs);
        return *this;
    }
public:
    auto GetVertex() const -> std::vector<T> const& {
        return _vertexes;
    }
    auto GetIndex() const -> std::vector<unsigned int> const& {
        return _index;
    }
    auto GetRenderData() const -> RenderData const& {
        return _renderData;
    }
    auto SetRenderData(RenderData renderData) -> void {
        _renderData = renderData;
    }
    auto GetRenderDataId() const -> unsigned int {
        return _renderDataId;
    }
    auto SetRenderDataId(unsigned int id) -> void {
        _renderDataId = id;
    }
private:
    std::vector<T> _vertexes;
    std::vector<unsigned int> _index;
    DrawMode _drawMode;

    RenderData _renderData;
private:
    unsigned int _renderDataId;
};

}
