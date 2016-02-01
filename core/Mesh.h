#pragma once

#include <vector>
#include <memory>

#include "Vertex.h"
#include "Resource.h"

namespace core {

class Mesh{
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
public:
	friend void swap(Mesh&, Mesh&);
    Mesh(DrawMode drawMode = triangles)
        : _drawMode(drawMode) {
    }
    Mesh(std::vector<Vertex> && vertexData, std::vector<unsigned int> && index);
    Mesh(std::vector<Vertex> && vertexData);
	Mesh(Mesh&& other);
	~Mesh();
	Mesh& operator=(Mesh);
	Mesh& operator=(Mesh && rhs);
public:
	auto GetVertex() const -> std::vector<Vertex> const& {
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
private:
	std::vector<Vertex> _vertexes;
	std::vector<unsigned int> _index;
    DrawMode _drawMode;

    RenderData _renderData;
public:
    unsigned int _renderDataId;
};

}
