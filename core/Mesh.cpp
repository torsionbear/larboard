#include "Mesh.h"

#include <GL/glew.h>

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

auto Mesh::SetVertexData(std::vector<Vertex>&& vertexData, vector<unsigned int> && index) -> void {
	_vertex = move(vertexData);
	_index = move(index);
}

auto Mesh::SetVertexData(std::vector<Vertex>&& vertexData) -> void {
	_vertex = move(vertexData);
	_index.resize(_vertex.size());
	for (auto i = 0u; i < _vertex.size(); ++i) {
		_index[i] = i;
	}
}

auto Mesh::Draw() -> void {
	glBindVertexArray(_vao);
	glDrawElementsBaseVertex(GL_TRIANGLES, _index.size(), GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(_indexOffset), _baseVertex);
}

}