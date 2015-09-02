#pragma once

#include <vector>

#include "Material.h"
#include "Texture.h"
#include "Mesh.h"
#include "Movable.h"
#include "Model.h"
#include "ShaderProgram.h"

namespace core {

class Shape {
	friend class Scene;

public:
	Shape(Model * model);

public:
	auto SetMaterial(Material * material) -> void;
	auto SetMesh(Mesh * mesh) -> void;
	auto AddTexture(Texture * texture) -> void;
	auto SetShaderProgram(ShaderProgram * shaderProgram) -> void;
	
private:
	Material * _material;
	Model * _model;
	Mesh * _mesh;
	std::vector<Texture *> _textures;
	ShaderProgram * _shaderProgram;
};


}