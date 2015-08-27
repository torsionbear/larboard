#include "Scene.h"

#include <algorithm>

#include <GL/glew.h>

using std::string;

namespace core {

Scene::Scene() = default;

Scene::~Scene() {
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);
}

auto Scene::CreateMovable() -> Movable * {
	_movables.emplace_back();
	return &_movables.back();
}

auto Scene::CreateModel() -> Model * {
	_models.emplace_back();
	return &_models.back();
}

auto Scene::CreateCamera() -> Camera * {
	_cameras.emplace_back();
	return &_cameras.back();
}

auto Scene::CreatePointLight() -> PointLight * {
	_pointLights.emplace_back();
	return &_pointLights.back();
}

auto Scene::Stage(Movable * movable) -> void {
	movable->AttachTo(_root);
}

auto Scene::Unstage(Movable * movable) -> void {
	movable->DetachFrom();
}

auto Scene::CreateShape(Model* model) -> Shape * {
	_shapes.emplace_back(model);
	return &_shapes.back();
}

auto Scene::CreateTexture(string const& filename) -> Texture* {
	// todo: make this factory method creating different textures according to different file type.
	_textures.emplace_back(filename);
	return &_textures.back();
}

auto Scene::CreateMesh() -> Mesh * {
	_meshes.emplace_back();
	return &_meshes.back();
}

auto Scene::CreateShaderProgram(string const& vertexShaderFile, string const& fragmentShaderFile) -> ShaderProgram * {
	_shaderProgram.emplace_back(vertexShaderFile, fragmentShaderFile);
	return &_shaderProgram.back();
}

auto Scene::CreateDefaultShaderProgram() -> ShaderProgram * {
	return CreateShaderProgram("default.vert", "default.frag");
}

auto Scene::SendToCard() -> void {
	for (auto & s : _shaderProgram) {
		s.SendToCard();
	}
	for (auto & t : _textures) {
		t.SendToCard();
	}

	// currently we use only 1 vao & vbo for a scene.
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	for (auto const & m : _meshes) {
		_vertexCount += m._vertex.size();
	}
	glBufferData(GL_ARRAY_BUFFER, _vertexCount * sizeof(Vertex), nullptr, GL_STATIC_DRAW);

	auto offset = 0;
	for (auto & m : _meshes) {
		m._vao = _vao;
		m._offset = offset * sizeof(Vertex);
		glBufferSubData(GL_ARRAY_BUFFER, m._offset, m._size, m._vertex.data());
		offset += m._vertex.size();
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
	Matrix4x4f viewTransform = camera.GetProjectionTransform() * camera.GetRigidBodyMatrixInverse();

	auto currentShaderProgram = static_cast<ShaderProgram*>(nullptr);
	for (auto const & s : _shapes) {
		if (currentShaderProgram != s._shaderProgram) {
			s._shaderProgram->Use();
			currentShaderProgram = s._shaderProgram;
			glUniformMatrix4fv(glGetUniformLocation(s._shaderProgram->GetHandler(), "viewTransform"),
				1, GL_TRUE, viewTransform.data());
		}

		error = glGetError();
		auto & worldTransform = s._model->GetMatrix();
		// opengl expect column major matrix, so we pass GL_TRUE to transpose our matrix.
		// another solution is to always multiply vector to matrix in shader (e.g. v_transformed = v * M)
		glUniformMatrix4fv(glGetUniformLocation(s._shaderProgram->GetHandler(), "worldTransform"), 
			1, GL_TRUE, worldTransform.data());

		error = glGetError();
		for (auto i = 0u; i < s._textures.size(); ++i) {
			s._textures[i]->Use(i);
			auto variable = string("texture").append({ static_cast<char>('0' + i) });
			glUniform1i(glGetUniformLocation(currentShaderProgram->GetHandler(), "texture0"), i);
		}
		glDrawArrays(GL_TRIANGLES, s._mesh->_offset, s._mesh->_size);
	}

	error = glGetError();
}

}