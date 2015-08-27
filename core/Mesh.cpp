#include "Mesh.h"

using std::vector;
using std::move;

namespace core {

void swap(Mesh& first, Mesh& second) {
	using std::swap;
	swap(first._vertex, second._vertex);
	swap(first._vao, second._vao);
}

Mesh::Mesh() = default;

Mesh::Mesh(Mesh const&) = default;

Mesh::Mesh(Mesh&& other)
	: Mesh() {
	swap(*this, other);
}

Mesh::~Mesh() = default;

Mesh& Mesh::operator=(Mesh rhs) {
	swap(*this, rhs);
	return *this;
}

Mesh& Mesh::operator=(Mesh && rhs) {
	swap(*this, rhs);
	return *this;
}

auto Mesh::SetVertexData(std::vector<Vertex>&& vertexData) -> void {
	_vertex = move(vertexData);
	_size = _vertex.size() * sizeof(Vertex);
}

}