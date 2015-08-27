#pragma once

#include <vector>
#include <memory>

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
	auto CreateTexture(std::string const&)->Texture *;
	auto CreateMesh()->Mesh *;
	auto CreateShaderProgram(std::string const& vertexShaderFile, std::string const& fragmentShaderFile)->ShaderProgram *;
	auto CreateDefaultShaderProgram()->ShaderProgram *;

	auto SendToCard() -> void;
	auto Draw() -> void;

private:
	Movable _root;
	std::vector<Movable> _movables;
	std::vector<Model> _models;
	std::vector<Camera> _cameras;
	std::vector<PointLight> _pointLights;

	std::vector<Shape> _shapes;
	std::vector<Texture> _textures;
	std::vector<Mesh> _meshes;
	std::vector<ShaderProgram> _shaderProgram;
	openglUint _vao;
	openglUint _vbo;
	size_t _vertexCount = 0;
};

}