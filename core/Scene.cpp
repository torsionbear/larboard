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
		_defaultShaderProgram = CreateShaderProgram("shader/default.vert", "shader/default.frag");
	}
	return _defaultShaderProgram;
}

auto Scene::SendToCard() -> void {

	// 0. setup ubo
	GenerateViewpointUbo();
	GenerateMaterialUbo();
	GenerateTransformUbo();

	// 1. shader program
	glEnable(GL_DEPTH_TEST);
	for (auto & s : _shaderProgram) {
		s->SendToCard();
	}

	// 2. texture
	for (auto & t : _textures) {
		t.second->SendToCard();
	}

	// 3. light


	// 4. vao/vbo
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

	// 0. feed shape independent data (viewpoint) to shader via ubo
	UpdateViewpoint(_cameras.front().get());

	auto currentShaderProgram = static_cast<ShaderProgram*>(nullptr);
	for (auto const& shape : _shapes) {
		// 1. switch shader program, set view transform & camera position, set lights, set texture
		if (currentShaderProgram != shape->_shaderProgram) {
			shape->_shaderProgram->Use();
			currentShaderProgram = shape->_shaderProgram;

			//glUniform1i(glGetUniformLocation(shape->_shaderProgram->GetHandler(), "lights.directionalLightCount"), 0);
			//glUniform1i(glGetUniformLocation(shape->_shaderProgram->GetHandler(), "lights.pointLightCount"), _pointLights.size());
			//glUniform1i(glGetUniformLocation(shape->_shaderProgram->GetHandler(), "lights.spotLightCount"), 0);
			for (auto const& pointLight : _pointLights) {

			}
			// Do all the glUniform1i calls after loading the program, then never again. 
			// You only need to call it once to tell the program which texture image unit each sampler uses. 
			// After you've done that all you need to do is bind textures to the right texture image units. 
			glUniform1i(glGetUniformLocation(currentShaderProgram->GetHandler(), "textures.diffuseTexture"), 0); // only support diffuse texture for now
		}

		// 2. feed shape dependent data (transform & material) to shader via ubo 
		UpdateTransform(shape->_model);
		UpdateMaterial(shape->_material);

		// 3. texture
		for (auto i = 0u; i < shape->_textures.size(); ++i) {
			shape->_textures[0]->Use(i);
		}

		// 4. draw
		glDrawArrays(GL_TRIANGLES, shape->_mesh->_startingIndex, shape->_mesh->_vertex.size());
	}
	error = glGetError();
}

auto Scene::GenerateViewpointUbo() -> void {
	glGenBuffers(1, &_viewpointUbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Viewpoint), _viewpointUbo);
}

auto Scene::GenerateTransformUbo() -> void {
	glGenBuffers(1, &_transformUbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Transform), _transformUbo);
}

auto Scene::GenerateMaterialUbo() -> void {
	glGenBuffers(1, &_materialUbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Material), _materialUbo);
}

auto Scene::UpdateViewpoint(Camera const* camera) -> void {
	glBindBuffer(GL_UNIFORM_BUFFER, _viewpointUbo);
	// Buffer re-specification (orphaning). See https://www.opengl.org/wiki/Buffer_Object_Streaming
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Viewpoint), nullptr, GL_DYNAMIC_DRAW);
	auto viewpoint = Viewpoint{
		camera->GetProjectionTransform() * camera->GetRigidBodyMatrixInverse(),
		camera->GetPosition(),
	};
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Viewpoint), &viewpoint);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::UpdateTransform(Model const* model) -> void {
	glBindBuffer(GL_UNIFORM_BUFFER, _transformUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Transform), nullptr, GL_DYNAMIC_DRAW);
	auto transform = Transform{
		model->GetMatrix(),
		model->GetNormalTransform(),
	};
	// 1. glsl expect column major matrix, but our matrix is row major, so we need to specify row_major in shader's interface block layout.
	// 2. when using glUniformMatrix4fv() to feed shader with matrix, we need to pass GL_TRUE for the 3rd parameter to transpose our matrix.
	// 3. another dirty solution is to always multiply vector to matrix in shader (e.g. v_transformed = v * M)
	// see http://stackoverflow.com/questions/17717600/confusion-between-c-and-opengl-matrix-order-row-major-vs-column-major#
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Transform), &transform);	
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::UpdateMaterial(Material const* material) -> void {
	glBindBuffer(GL_UNIFORM_BUFFER, _materialUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), material);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

}