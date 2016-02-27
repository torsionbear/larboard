#pragma once

#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

#include "Model.h"
#include "Shape.h"
#include "Bvh.h"

namespace core {

class StaticModelGroup {
public:
    auto Load() -> void;
    auto GetShapes()->std::vector<std::unique_ptr<Shape>>&;
    auto AcquireShapes()->std::vector<std::unique_ptr<Shape>>;
    auto AcquireMeshes()->std::vector<std::unique_ptr<Mesh>>;

    auto CreateMovable()->Movable *;
    auto CreateModel()->Model *;
    auto CreateShape(Model *)->Shape *;
    auto CreateMaterial() -> Material * {
        _materials.emplace_back(std::make_unique<Material>());
        return _materials.back().get();
    }
    auto AddMaterial(std::unique_ptr<Material> && material) -> Material * {
        _materials.push_back(move(material));
        return _materials.back().get();
    }
    auto CreateTexture(std::string const& filename, TextureUsage::TextureType type = TextureUsage::DiffuseMap) -> Texture * {
        _textures.emplace_back(std::make_unique<Texture>(filename, type));
        return _textures.back().get();
    }
    template <typename... Args>
    auto CreateMesh(Args&&... args) -> Mesh * {
        _meshes.push_back(std::make_unique<Mesh>(std::forward<Args>(args)...));
        return _meshes.back().get();
    }
    auto CreateShaderProgram(std::string name, std::string const& vertexShaderFile, std::string const& fragmentShaderFile)->ShaderProgram *;
    auto GetShaderProgram(std::string name) const->ShaderProgram*;
    auto GetBvh() -> Bvh * {
        return _bvh.get();
    }
    auto GetMeshes() -> std::vector<std::unique_ptr<Mesh>> const& {
        return _meshes;
    }
    auto GetModels() ->std::vector<std::unique_ptr<Model>> const& {
        return _models;
    }

public:
    Movable _root;
    std::vector<std::unique_ptr<Movable>> _movables;
    std::vector<std::unique_ptr<Model>> _models;
    std::vector<std::unique_ptr<Shape>> _shapes;
    std::vector<std::unique_ptr<Texture>> _textures;
    std::vector<std::unique_ptr<Material>> _materials;
    std::vector<std::unique_ptr<Mesh>> _meshes;
    std::unordered_map<std::string, std::unique_ptr < ShaderProgram >> _shaderProgram;
    std::unique_ptr<Bvh> _bvh = nullptr;

    size_t _vertexCount = 0;
};

}