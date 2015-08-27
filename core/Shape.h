#pragma once

#include <vector>

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
	auto SetDiffuse(Vector3f) -> void;
	auto SetSpecular(Vector3f) -> void;
	auto SetEmissive(Vector3f) -> void;
	auto SetAmbientIntensity(Float32) -> void;
	auto SetShininess(Float32) -> void;
	auto SetTransparency(Float32) -> void;

	auto SetMesh(Mesh *) -> void;
	auto AddTexture(Texture * texture) -> void;
	auto SetShaderProgram(ShaderProgram *) -> void;
	
private:
	Model * _model;
	Mesh * _mesh;
	std::vector<Texture *> _textures;
	ShaderProgram * _shaderProgram;
	Vector3f _diffuse;
	Vector3f _specular;
	Vector3f _emissive;
	Float32 _ambientIntensity;
	Float32 _shininess;
	Float32 _transparency;
};


}