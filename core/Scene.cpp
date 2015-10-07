#include "Scene.h"

#include <algorithm>

#include <GL/glew.h>

using std::string;
using std::make_unique;

namespace core {

Scene::Scene() {
	// There is an alignment restriction for UBOs when binding. 
	// Any glBindBufferRange/Base's offset must be a multiple of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT. 
	// This alignment could be anything, so you have to query it before building your array of uniform buffers. 
	// That means you can't do it directly in compile-time C++ logic; it has to be runtime logic.
	// see http://stackoverflow.com/questions/13028852/issue-with-glbindbufferrange-opengl-3-1
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_uboAlignment);
	_cameraShaderDataSize = AlignedSize(_uboAlignment, sizeof(Camera::ShaderData));
	_materialShaderDataSize = AlignedSize(_uboAlignment, sizeof(Material::ShaderData));
	_transformShaderDataSize = AlignedSize(_uboAlignment, sizeof(Movable::ShaderData));
}

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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// 0. setup ubo
	InitCameraData();
	InitTransformData();
	LoadMaterialData();
	LoadLightData();

	// 1. shader program
	for (auto & s : _shaderProgram) {
		s->SendToCard();
	}

	// 2. texture
	for (auto & t : _textures) {
		t.second->SendToCard();
	}

	// 3. vao/vbo
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

	// 0. feed model independent data (camera) to shader via ubo
	LoadCameraData();
	UseCameraData(_cameras.front().get());

	// 1. prepare transform data
	LoadTransformData();

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
		UseTransformData(shape->_model);
		UseMaterialData(shape->_material);
		
		// 3. texture
		for (auto i = 0u; i < shape->_textures.size(); ++i) {
			shape->_textures[0]->Use(i);
		}

		// 4. feed vertex data via vao, draw call
		glBindVertexArray(_vao);
		glDrawArrays(GL_TRIANGLES, shape->_mesh->_startingIndex, shape->_mesh->_vertex.size());
	}
	error = glGetError();
}

// store all camera's data in _cameraUbo. 
// if there's not need to split screen for multiple player, 
// maybe we should store only 1 camera's data in _cameraUbo
auto Scene::InitCameraData() -> void {
	glGenBuffers(1, &_cameraUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _cameraUbo);
	glBufferData(GL_UNIFORM_BUFFER, _cameraShaderDataSize * _cameras.size(), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::LoadCameraData() -> void {
	auto cache = vector<unsigned char>(_cameraShaderDataSize * _cameras.size());
	auto offset = 0;
	for (auto & camera : _cameras) {
		auto * p = reinterpret_cast<Camera::ShaderData *>(&cache[offset]);
		*p = camera->GetShaderData();
		camera->SetUboOffset(offset);
		offset += _cameraShaderDataSize;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, _cameraUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, cache.size(), cache.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::UseCameraData(Camera const * camera) -> void {
	glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Camera), _cameraUbo, camera->GetUboOffset(), sizeof(Camera::ShaderData));
}

auto Scene::InitTransformData() -> void {
	glGenBuffers(1, &_transformUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _transformUbo);
	glBufferData(GL_UNIFORM_BUFFER, _transformShaderDataSize * _shapes.size(), nullptr, GL_DYNAMIC_DRAW);
	// may use glBufferStorage() instead of glBufferData() on gl version 4.4+
	//glBufferStorage(GL_UNIFORM_BUFFER, _transformShaderDataSize * _shapes.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::LoadTransformData() -> void {
	auto cache = vector<unsigned char>(_transformShaderDataSize * _models.size());
	auto offset = 0;
	for (auto & model : _models) {
		auto * p = reinterpret_cast<Movable::ShaderData *>(cache.data() + offset);
		*p = model->GetShaderData();
		model->SetUboOffset(offset);
		offset += _transformShaderDataSize;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, _transformUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, cache.size(), cache.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::UseTransformData(Model const* model) -> void {
	glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Transform), _transformUbo, model->GetUboOffset(), _transformShaderDataSize);
}

auto Scene::LoadMaterialData() -> void {
	glGenBuffers(1, &_materialUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _materialUbo);
	glBufferData(GL_UNIFORM_BUFFER, _materialShaderDataSize * _materials.size(), nullptr, GL_DYNAMIC_DRAW);
	auto * p = static_cast<unsigned char*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
	auto offset = 0;
	for (auto & m : _materials) {
		auto * material = m.second.get();
		material->SetUboOffset(offset);
		memcpy(p, &material->GetShaderData(), sizeof(Material::ShaderData));
		p += _materialShaderDataSize;
		offset += _materialShaderDataSize;
	}
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::UseMaterialData(Material const* material) -> void {
	glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Material), _materialUbo, material->GetUboOffset(), _materialShaderDataSize);
}

auto Scene::LoadLightData() -> void {
	auto data = LightShaderData{};

	data.directionalLightCount = 0;
	assert(data.directionalLightCount <= LightShaderData::MaxDirectionalLightCount);

	data.pointLightCount = _pointLights.size();
	assert(data.pointLightCount <= LightShaderData::MaxpointLightCount);
	auto index = 0u;
	for (auto const& pointLight : _pointLights) {
		data.pointLights[index++] = pointLight->GetShaderData();
	}

	data.spotLightCount = 0;
	assert(data.spotLightCount <= LightShaderData::MaxspotLightCount);

	glGenBuffers(1, &_lightUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _lightUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &data, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Light), _lightUbo);
}

}