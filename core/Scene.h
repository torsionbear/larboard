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

namespace core {

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
	auto CreatePointLight()->PointLight *;
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

	auto SendToCard() -> void;
	auto Draw() -> void;

private:
	Movable _root;
	std::vector<std::unique_ptr<Movable>> _movables;
	std::vector<std::unique_ptr<Model>> _models;
	std::vector<std::unique_ptr<Camera>> _cameras;
	std::vector<std::unique_ptr<PointLight>> _pointLights;

	std::vector<std::unique_ptr<Shape>> _shapes;
	std::map<std::string, std::unique_ptr<Material>> _materials;
	std::map<std::string, std::unique_ptr<Texture>> _textures;
	std::vector<std::unique_ptr<Mesh>> _meshes;
	std::vector<std::unique_ptr<ShaderProgram>> _shaderProgram;
	ShaderProgram * _defaultShaderProgram = nullptr;

	openglUint _vao;
	openglUint _vbo;
	openglUint _ubo;
	size_t _vertexCount = 0;
};

}