#include "StaticModelGroup.h"

using std::make_unique;
using std::string;

namespace core {

StaticModelGroup::~StaticModelGroup() {
}

auto StaticModelGroup::PrepareForDraw() -> void {
    // 0. setup ubo
    LoadTransformData();
    LoadMaterialData();

    // 1. shader program
    for (auto & s : _shaderProgram) {
        s.second->SendToCard();
    }

    // 2. texture
    for (auto & t : _textures) {
        t.second->Load();
        _resourceManager->LoadTexture(t.second.get());
    }

    // 3. vao/vbo
    _resourceManager->LoadMeshes(_meshes);

    // 4. build BVH
    auto shapes = vector<Shape *>{};
    for (auto const& s : _shapes) {
        shapes.push_back(s.get());
    }
    _bvh = make_unique<Bvh>(move(shapes));
}

auto StaticModelGroup::GetShapes() -> std::vector<std::unique_ptr<Shape>>& {
    return _shapes;
}

auto StaticModelGroup::AcquireShapes() -> std::vector<std::unique_ptr<Shape>> {
    return move(_shapes);
}

auto StaticModelGroup::AcquireMeshes() -> std::vector<std::unique_ptr<Mesh>> {
    return move(_meshes);
}

auto StaticModelGroup::CreateMovable() -> Movable * {
    _movables.push_back(make_unique<Movable>());
    return _movables.back().get();
}

auto StaticModelGroup::CreateModel() -> Model * {
    _models.push_back(make_unique<Model>());
    return _models.back().get();
}

auto StaticModelGroup::CreateShape(Model* model) -> Shape * {
    _shapes.push_back(make_unique<Shape>(model));
    return _shapes.back().get();
}

auto StaticModelGroup::CreateMaterial(std::string const & materialName) -> Material * {
    auto newMaterial = make_unique<Material>();
    auto ret = newMaterial.get();
    _materials[materialName] = move(newMaterial);
    return ret;
}

auto StaticModelGroup::CreateTexture(string const& textureName, string const& filename) -> Texture * {
    auto newTexture = make_unique<Texture>(filename);
    auto ret = newTexture.get();
    _textures[textureName] = move(newTexture);
    return ret;
}

auto StaticModelGroup::GetShaderProgram(std::string name) const -> ShaderProgram * {
    if (_shaderProgram.find(name) == _shaderProgram.end()) {
        return nullptr;
    }
    return _shaderProgram.at(name).get();
}

auto StaticModelGroup::CreateShaderProgram(string name, string const& vertexShaderFile, string const& fragmentShaderFile) -> ShaderProgram * {
    _shaderProgram[name] = make_unique<ShaderProgram>(vertexShaderFile, fragmentShaderFile);
    return _shaderProgram.at(name).get();
}

auto StaticModelGroup::GetMaterial(std::string const & materialName) const -> Material * {
    return _materials.at(materialName).get();
}

auto StaticModelGroup::GetTexture(std::string const & textureName) const -> Texture * {
    return _textures.at(textureName).get();
}

auto StaticModelGroup::LoadTransformData() -> void {
    auto models = vector<Model *>();
    for (auto & model : _models) {
        models.push_back(model.get());
    }
    _resourceManager->LoadModels(models);
}

auto StaticModelGroup::LoadMaterialData() -> void {
    auto materials = vector<Material *>();
    for (auto & m : _materials) {
        materials.push_back(m.second.get());
    }
    _resourceManager->LoadMaterials(materials);
}

}
