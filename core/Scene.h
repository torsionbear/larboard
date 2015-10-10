#pragma once

#include <vector>
#include <memory>
#include <map>

#include "SceneNode.h"
#include "Camera.h"
#include "Movable.h"
#include "Model.h"
#include "Shape.h"
#include "Camera.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"

namespace core {

struct LightShaderData {
	enum {
		MaxDirectionalLightCount = 10,
		MaxPointLightCount = 50,
		MaxSpotLightCount = 50,
	};
	DirectionalLight::ShaderData directionalLights[MaxDirectionalLightCount];
	PointLight::ShaderData pointLights[MaxPointLightCount];
	SpotLight::ShaderData spotLights[MaxSpotLightCount];
	int directionalLightCount;
	int pointLightCount;
	int spotLightCount;
};

class Scene {
public:
	Scene();
	Scene(Scene const&) = delete;
	Scene& operator=(Scene const&) = delete;
	~Scene();
public:
	auto CreateMovable() -> Movable *;
	auto CreateModel() -> Model *;
	auto CreateCamera() -> Camera *;
	auto CreateDirectionalLight() -> DirectionalLight *;
	auto CreatePointLight() -> PointLight *;
	auto CreateSpotLight() -> SpotLight *;
	auto Stage(Movable *) -> void;
	auto Unstage(Movable *) -> void;

	auto CreateShape(Model *) -> Shape *;
	auto CreateMaterial(std::string const& materialName) -> Material *;
	auto CreateTexture(std::string const& textureName, std::string const& filename) -> Texture *;
	auto CreateMesh() -> Mesh *;
	auto CreateShaderProgram(std::string const& vertexShaderFile, std::string const& fragmentShaderFile) -> ShaderProgram *;

	auto GetMaterial(std::string const& materialName) const->Material *;
	auto GetTexture(std::string const& textureName) const -> Texture *;
	auto GetActiveCamera() const -> Camera *;
	auto GetDefaultShaderProgram()->ShaderProgram *;

	auto Scene::ToggleBackFace() -> void;
	auto Scene::ToggleWireframe() -> void;
	auto SendToCard() -> void;
	auto Draw() -> void;

private:
	auto InitCameraData() -> void;
	auto LoadCameraData() -> void;
	auto UseCameraData(Camera const* camera) -> void;
	auto InitTransformData() -> void;
	auto LoadTransformData() -> void;
	auto UseTransformData(Model const* model) -> void;
	auto LoadMaterialData() -> void;
	auto UseMaterialData(Material const* material) -> void;
	auto LoadLightData() -> void;

private:
	Movable _root;
	std::vector<std::unique_ptr<Movable>> _movables;
	std::vector<std::unique_ptr<Model>> _models;
	std::vector<std::unique_ptr<Camera>> _cameras;
	std::vector<std::unique_ptr<PointLight>> _pointLights;
	std::vector<std::unique_ptr<DirectionalLight>> _directionalLights;
	std::vector<std::unique_ptr<SpotLight>> _spotLights;
	std::vector<std::unique_ptr<Shape>> _shapes;
	std::map<std::string, std::unique_ptr<Material>> _materials;
	std::map<std::string, std::unique_ptr<Texture>> _textures;
	std::vector<std::unique_ptr<Mesh>> _meshes;
	std::vector<std::unique_ptr<ShaderProgram>> _shaderProgram;
	ShaderProgram * _defaultShaderProgram = nullptr;

	openglInt _uboAlignment;
	unsigned int _cameraShaderDataSize;
	unsigned int _materialShaderDataSize;
	unsigned int _transformShaderDataSize;
	openglUint _vao;
	openglUint _vbo;
	openglUint _cameraUbo;
	openglUint _transformUbo;
	openglUint _materialUbo;
	openglUint _lightUbo;
	size_t _vertexCount = 0;

	bool _wireframeMode = false;
	bool _renderBackFace = false;
};

}