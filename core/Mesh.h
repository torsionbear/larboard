#pragma once

#include <vector>
#include <memory>

#include "Vertex.h"
#include "Resource.h"

namespace core {

class Mesh{
public:
	friend void swap(Mesh&, Mesh&);
	Mesh();
	Mesh(Mesh const&);
	Mesh(Mesh&& other);
	~Mesh();
	Mesh& operator=(Mesh);
	Mesh& operator=(Mesh && rhs);
	
public:
	auto SetVertexData(std::vector<Vertex> && vertexData, std::vector<unsigned int> && index) -> void;
	auto SetVertexData(std::vector<Vertex> && vertexData) -> void;
	auto GetVertex() -> std::vector<Vertex> const& {
		return _vertex;
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
	auto Draw() -> void;

private:
	std::vector<Vertex> _vertex;
	std::vector<unsigned int> _index;
	openglUint _vao;
	openglUint _indexOffset;	// represents an offset, in bytes, into the element array buffer where the indices begin
	openglInt _baseVertex;	// last paramerter for glDrawElementsBaseVertex()
};

}
