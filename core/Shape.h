#pragma once

#include <vector>
#include <memory>

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
    auto GetMaterial() const -> Material const* {
        return _material;
    }
    auto GetMesh() const -> Mesh const* {
        return _mesh;
    }
    auto GetShaderProgram() const -> ShaderProgram const * {
        return _shaderProgram;
    }
    auto GetModel() const -> Model const* {
        return _model;
    }
    auto GetTextures() const -> std::vector<Texture *> const& {
        return _textures;
    }
    auto AddTexture(Texture * texture) -> void {
        _textures.push_back(texture);
    }
    auto GetAabb() -> Aabb const& {
        // todo: need to update Aabb when shape's model is transformed
        if (_aabb == nullptr) {
            _aabb = std::make_unique<Aabb>();
            for (auto const& vertex : _mesh->GetVertex()) {
                auto transformedVertex = _model->GetTransform() * Point4f { vertex.coord(0), vertex.coord(1), vertex.coord(2), 1.0f };
                _aabb->Expand(transformedVertex);
            }
        }
        return *_aabb;
    }
private:
	Material * _material;
	Model * _model;
	Mesh * _mesh;
	std::vector<Texture *> _textures;
	ShaderProgram * _shaderProgram;
    std::unique_ptr<Aabb> _aabb = nullptr;
};


}