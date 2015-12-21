#pragma once

#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

#include "Renderer.h"
#include "Model.h"
#include "Shape.h"
#include "Bvh.h"

namespace core {

class StaticModelGroup {
public:
    StaticModelGroup(ResourceManager* resourceManager, Renderer * renderer)
        : _resourceManager(resourceManager)
        , _renderer(renderer) {
    }
    ~StaticModelGroup();
public:
    auto PrepareForDraw() -> void;
    auto Draw() -> void;
    auto GetShapes() -> std::vector<std::unique_ptr<Shape>>&;
    auto AcquireShapes() -> std::vector<std::unique_ptr<Shape>>;
    auto AcquireMeshes() -> std::vector<std::unique_ptr<Mesh>>;

    auto CreateMovable()->Movable *;
    auto CreateModel()->Model *;
    auto CreateShape(Model *)->Shape *;
    auto CreateMaterial(std::string const& materialName)->Material *;
    auto CreateTexture(std::string const& textureName, std::string const& filename)->Texture *;
    template <typename... Args>
    auto CreateMesh(Args&&... args) -> Mesh * {
        _meshes.push_back(make_unique<Mesh>(std::forward<Args>(args)...));
        return _meshes.back().get();
    }
    auto CreateShaderProgram(std::string name, std::string const& vertexShaderFile, std::string const& fragmentShaderFile) -> ShaderProgram *;
    auto GetShaderProgram(std::string name) const -> ShaderProgram*;
    auto GetMaterial(std::string const& materialName) const -> Material *;
    auto GetTexture(std::string const& textureName) const -> Texture *;
    auto GetBvh() -> Bvh * {
        return _bvh.get();
    }

private:
    auto LoadTransformData() -> void;
    auto LoadMaterialData() -> void;

private:
    ResourceManager * _resourceManager;
    Renderer * _renderer;
    Movable _root;
    std::vector<std::unique_ptr<Movable>> _movables;
    std::vector<std::unique_ptr<Model>> _models;
    std::vector<std::unique_ptr<Shape>> _shapes;
    std::map<std::string, std::unique_ptr<Material>> _materials;
    std::map<std::string, std::unique_ptr<Texture>> _textures;
    std::vector<std::unique_ptr<Mesh>> _meshes;
    std::unordered_map<std::string, std::unique_ptr < ShaderProgram >> _shaderProgram;
    std::unique_ptr<Bvh> _bvh = nullptr;

    size_t _vertexCount = 0;
};

}