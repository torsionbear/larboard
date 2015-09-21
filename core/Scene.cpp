#include "Scene.h"

#include <algorithm>

#include <GL/glew.h>

using std::string;
using std::make_unique;

namespace core {

Scene::Scene() = default;

Scene::~Scene() {
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);
}

auto Scene::CreateMovable() -> Movable * {
	_movables.push_back(make_unique<Movable>());
	return _movables.back().get();
}

auto Scene::CreateModel() -> Model * {
	_models.push_back(make_unique<Model>());
	return _models.back().get();
}

auto Scene::CreateCamera() -> Camera * {
	_cameras.push_back(make_unique<Camera>());
	return _cameras.back().get();
}

auto Scene::CreatePointLight() -> PointLight * {
	_pointLights.push_back(make_unique<PointLight>());
	return _pointLights.back().get();
}

auto Scene::Stage(Movable * movable) -> void {
	movable->AttachTo(_root);
}

auto Scene::Unstage(Movable * movable) -> void {
	movable->DetachFrom();
}

auto Scene::CreateShape(Model* model) -> Shape * {
	_shapes.push_back(make_unique<Shape>(model));
	return _shapes.back().get();
}

auto Scene::CreateMaterial(std::string const & materialName) -> Material * {
	auto newMaterial = make_unique<Material>();
	auto ret = newMaterial.get();
	_materials[materialName] = move(newMaterial);
	return ret;
}

auto Scene::CreateTexture(string const& textureName, string const& filename) -> Texture * {
	auto newTexture = make_unique<Texture>(filename);
	auto ret = newTexture.get();
	_textures[textureName] = move(newTexture);
	return ret;
}

auto Scene::CreateMesh() -> Mesh * {
	_meshes.push_back(make_unique<Mesh>());
	return _meshes.back().get();
}

auto Scene::CreateShaderProgram(string const& vertexShaderFile, string const& fragmentShaderFile) -> ShaderProgram * {
	_shaderProgram.push_back(make_unique<ShaderProgram>(vertexShaderFile, fragmentShaderFile));
	return _shaderProgram.back().get();
}

auto Scene::GetMaterial(std::string const & materialName) const -> Material * {
	return _materials.at(materialName).get();
}

auto Scene::GetTexture(std::string const & textureName) const -> Texture * {
	return _textures.at(textureName).get();
}

auto Scene::GetActiveCamera() const -> Camera * {
	return _cameras.front().get();
}

auto Scene::GetDefaultShaderProgram() -> ShaderProgram * {
	if (_defaultShaderProgram == nullptr) {
		_defaultShaderProgram = CreateShaderProgram("default.vert", "default.frag");
	}
	return _defaultShaderProgram;
}

auto Scene::SendToCard() -> void {
	glEnable(GL_DEPTH_TEST);
	for (auto & s : _shaderProgram) {
		s->SendToCard();
	}
	for (auto & t : _textures) {
		t.second->SendToCard();
	}

	// currently we use only 1 vao & vbo for a scene.
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	for (auto const& m : _meshes) {
		_vertexCount += m->_vertex.size();
	}
	glBufferData(GL_ARRAY_BUFFER, _vertexCount * sizeof(Vertex), nullptr, GL_STATIC_DRAW);

	auto vertexCount = 0;
	for (auto & m : _meshes) {
		m->_vao = _vao;
		m->_startingIndex = vertexCount;
		glBufferSubData(GL_ARRAY_BUFFER, m->_startingIndex * sizeof(Vertex), m->_vertex.size() * sizeof(Vertex), m->_vertex.data());
		vertexCount += m->_vertex.size();
	}

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(Vector3f)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(Vector3f)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	// todo: sort shapes according to: 1. shader priority; 2. vbo/vao
}

auto Scene::Draw() -> void {
	auto error = glGetError();
	glBindVertexArray(_vao);

	auto & camera = _cameras.front();	
	Matrix4x4f viewTransform = camera->GetProjectionTransform() * camera->GetRigidBodyMatrixInverse();

	auto currentShaderProgram = static_cast<ShaderProgram*>(nullptr);
	for (auto const& s : _shapes) {
		if (currentShaderProgram != s->_shaderProgram) {
			s->_shaderProgram->Use();
			currentShaderProgram = s->_shaderProgram;
			glUniformMatrix4fv(glGetUniformLocation(s->_shaderProgram->GetHandler(), "viewTransform"),
				1, GL_TRUE, viewTransform.data());
		}

		auto & worldTransform = s->_model->GetMatrix();
		// opengl expect column major matrix, so we pass GL_TRUE to transpose our matrix.
		// another solution is to always multiply vector to matrix in shader (e.g. v_transformed = v * M)
		// see http://stackoverflow.com/questions/17717600/confusion-between-c-and-opengl-matrix-order-row-major-vs-column-major#
		glUniformMatrix4fv(glGetUniformLocation(s->_shaderProgram->GetHandler(), "worldTransform"),
			1, GL_TRUE, worldTransform.data());

		for (auto i = 0u; i < s->_textures.size(); ++i) {
			s->_textures[i]->Use(i);
			auto variable = string("texture").append({ static_cast<char>('0' + i) });
			glUniform1i(glGetUniformLocation(currentShaderProgram->GetHandler(), variable.c_str()), i);
		}
		glDrawArrays(GL_TRIANGLES, s->_mesh->_startingIndex, s->_mesh->_vertex.size());
	}
	error = glGetError();
}

}