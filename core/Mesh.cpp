#include "Mesh.h"

#include <limits>

using std::vector;
using std::move;
using std::make_unique;

namespace core {

void swap(Mesh& first, Mesh& second) {
    using std::swap;
    swap(first._vertexes, second._vertexes);
    swap(first._renderData, second._renderData);
}

Mesh::Mesh(std::vector<Vertex>&& vertexData, std::vector<unsigned int>&& index)
    : _vertexes(move(vertexData))
    , _index(move(index)) {
}

Mesh::Mesh(std::vector<Vertex>&& vertexData)
    : _vertexes(move(vertexData)) {
    _index.resize(_vertexes.size());
    for (auto i = 0u; i < _vertexes.size(); ++i) {
        _index[i] = i;
    }
}

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

}