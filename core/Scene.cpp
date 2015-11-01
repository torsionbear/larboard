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

	// todo: send in resourceManager by argument
	_resourceManager = make_unique<ResourceManager>();
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

auto Scene::CreateDirectionalLight() -> DirectionalLight * {
	_directionalLights.push_back(make_unique<DirectionalLight>());
	return _directionalLights.back().get();
}

auto Scene::CreatePointLight() -> PointLight * {
	_pointLights.push_back(make_unique<PointLight>());
	return _pointLights.back().get();
}

auto Scene::CreateSpotLight() -> SpotLight * {
	_spotLights.push_back(make_unique<SpotLight>());
	return _spotLights.back().get();
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

auto Scene::ToggleBackFace() -> void {
	_renderBackFace = !_renderBackFace;
	_renderBackFace ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
}

auto Scene::ToggleWireframe() -> void {
	_wireframeMode = !_wireframeMode;
	_wireframeMode ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	_resourceManager->LoadMeshes(_meshes);

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
		}

		// 2. feed shape dependent data (transform & material) to shader via ubo 
		UseTransformData(shape->_model);
		UseMaterialData(shape->_material);
		
		// 3. texture
		for (auto & texture : shape->_textures) {
			texture->Use();
		}

		// 4. feed vertex data via vao, draw call
		shape->_mesh->Draw();
	}
	error = glGetError();
}

auto Scene::GetShapes() -> std::vector<std::unique_ptr<Shape>>& {
	return _shapes;
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

	data.directionalLightCount = _directionalLights.size();
	assert(data.directionalLightCount <= LightShaderData::MaxDirectionalLightCount);
	for (auto i = 0; i < data.directionalLightCount; ++i) {
		data.directionalLights[i] = _directionalLights[i]->GetShaderData();
	}

	data.pointLightCount = _pointLights.size();
	assert(data.pointLightCount <= LightShaderData::MaxPointLightCount);
	for (auto i = 0; i < data.pointLightCount; ++i) {
		data.pointLights[i] = _pointLights[i]->GetShaderData();
	}

	data.spotLightCount = _spotLights.size();
	assert(data.spotLightCount <= LightShaderData::MaxSpotLightCount);
	for (auto i = 0; i < data.spotLightCount; ++i) {
		data.spotLights[i] = _spotLights[i]->GetShaderData();
	}

	glGenBuffers(1, &_lightUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _lightUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &data, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Light), _lightUbo);
}

}