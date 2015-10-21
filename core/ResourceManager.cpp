#include "ResourceManager.h"

#include <vector>

#include <GL/glew.h>

using std::vector;

namespace core {

auto ResourceManager::LoadMeshes(std::vector<std::unique_ptr<Mesh>> const& meshes) -> void {
	_vertexArrayObjects.push_back(0);
	auto & vao = _vertexArrayObjects.back();
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	_vertexBufferObjects.push_back(0);
	auto & vbo = _vertexBufferObjects.back();
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	_vertexElementObjects.push_back(0);
	auto & veo = _vertexElementObjects.back();
	glGenBuffers(1, &veo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);

	auto vertexData = vector<Vertex>();
	auto indexData = vector<unsigned int>();
	for (auto const& mesh : meshes) {
		mesh->SetVertexArrayObject(vao);
		mesh->SetBaseVertex(vertexData.size());
		mesh->SetIndexOffset(indexData.size() * sizeof(unsigned int));
		vertexData.insert(vertexData.end(), mesh->GetVertex().cbegin(), mesh->GetVertex().cend());
		indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
	}
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(Vector3f)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(Vector3f)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

}
