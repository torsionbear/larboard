#pragma once

#include <vector>

#include "Material.h"
#include "Texture.h"
#include "Mesh.h"
#include "Movable.h"
#include "Model.h"
#include "ShaderProgram.h"
#include "Aabb.h"

namespace core {

class Shape {
	friend class Scene;
public:
	Shape(Model * model)
        : _model(model) {
    }
public:
	auto SetMaterial(Material * material) -> void {
        _material = material;
    }
	auto SetMesh(Mesh * mesh) -> void {
        _mesh = mesh;
    }
	auto SetShaderProgram(ShaderProgram * shaderProgram) -> void {
        _shaderProgram = shaderProgram;
    }
    auto GetMesh() const -> Mesh const* {
        return _mesh;
    }
    auto GetModel() const -> Model const* {
        return _model;
    }
    auto AddTexture(Texture * texture) -> void;
    auto GetAabb() -> Aabb const&;
private:
	Material * _material;
	Model * _model;
	Mesh * _mesh;
	std::vector<Texture *> _textures;
	ShaderProgram * _shaderProgram;
    std::unique_ptr<Aabb> _aabb = nullptr;
};


}