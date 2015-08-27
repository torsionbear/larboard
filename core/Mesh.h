#pragma once

#include <vector>

#include "Primitive.h"
#include "Vertex.h"
#include "Resource.h"

namespace core {

class Mesh{
public:
	friend class Scene;

public:
	friend void swap(Mesh&, Mesh&);
	Mesh();
	Mesh(const Mesh&);
	Mesh(Mesh&& other);
	~Mesh();
	Mesh& operator=(Mesh);
	Mesh& operator=(Mesh && rhs);
	
public:
	auto SetVertexData(std::vector<Vertex> &&) -> void;

private:
	std::vector<Vertex> _vertex;
	openglUint _vao;
	openglInt _offset;
	openglSizei _size;
};

}
