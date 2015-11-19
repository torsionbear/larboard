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
public:
	friend void swap(Mesh&, Mesh&);
    Mesh() = default;
    Mesh(std::vector<Vertex> && vertexData, std::vector<unsigned int> && index);
    Mesh(std::vector<Vertex> && vertexData);
	Mesh(Mesh&& other);
	~Mesh();
	Mesh& operator=(Mesh);
	Mesh& operator=(Mesh && rhs);
public:
	auto GetVertex() -> std::vector<Vertex> const& {
		return _vertexes;
	}
	auto GetIndex() -> std::vector<unsigned int> const& {
		return _index;
	}
	auto SetVertexArrayObject(openglUint vao) -> void {
		_vao = vao;
	}
	auto SetBaseVertex(openglInt baseVertex) -> void {
		_baseVertex = baseVertex;
	}
	auto SetIndexOffset(openglUint indexOffset) -> void {
		_indexOffset = indexOffset;
	}
	auto Draw(DrawMode drawMode = triangles) const -> void;
private:
	std::vector<Vertex> _vertexes;
	std::vector<unsigned int> _index;
	openglUint _vao;
	openglUint _indexOffset;	// represents an offset, in bytes, into the element array buffer where the indices begin
	openglInt _baseVertex;	// last paramerter for glDrawElementsBaseVertex()
};

}
