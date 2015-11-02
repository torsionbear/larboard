#include "StaticModelGroup.h"

#include <GL/glew.h>

using std::make_unique;
using std::string;

namespace core {

StaticModelGroup::~StaticModelGroup() {
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_veo);
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_transformUbo);
    glDeleteBuffers(1, &_materialUbo);
}

auto StaticModelGroup::PrepareForDraw() -> void {
    
    // 0. setup ubo
    InitTransformData();
    LoadMaterialData();

    // 1. shader program
    for (auto & s : _shaderProgram) {
        s->SendToCard();
    }

    // 2. texture
    for (auto & t : _textures) {
        t.second->SendToCard();
    }

    // 3. vao/vbo
    _resourceManager->LoadMeshes(_meshes);

    // 4. build BVH
    auto shapes = vector<Shape *>{};
    for (auto const& s : _shapes) {
        shapes.push_back(s.get());
    }
    _bvh = make_unique<Bvh>(move(shapes));

    _bvh->PrepareForDraw(*_resourceManager);

    // todo: sort shapes according to: 1. shader priority; 2. vbo/vao
}

auto StaticModelGroup::Draw() -> void {
    // 1. prepare transform data
    LoadTransformData();

    auto currentShaderProgram = static_cast<ShaderProgram const*>(nullptr);
    for (auto const& shape : _shapes) {
        // 1. switch shader program, set view transform & camera position, set lights, set texture
        if (currentShaderProgram != shape->GetShaderProgram()) {
            shape->GetShaderProgram()->Use();
            currentShaderProgram = shape->GetShaderProgram();
        }

        // 2. feed shape dependent data (transform & material) to shader via ubo 
        UseTransformData(shape->GetModel());
        UseMaterialData(shape->GetMaterial());

        // 3. texture
        for (auto & texture : shape->GetTextures()) {
            texture->Use();
        }

        // 4. feed vertex data via vao, draw call
        shape->GetMesh()->Draw();
    }

    _bvh->Draw();
}

auto StaticModelGroup::GetShapes() -> std::vector<std::unique_ptr<Shape>>& {
    return _shapes;
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

auto StaticModelGroup::CreateShaderProgram(string const& vertexShaderFile, string const& fragmentShaderFile) -> ShaderProgram * {
    _shaderProgram.push_back(make_unique<ShaderProgram>(vertexShaderFile, fragmentShaderFile));
    return _shaderProgram.back().get();
}

auto StaticModelGroup::GetMaterial(std::string const & materialName) const -> Material * {
    return _materials.at(materialName).get();
}

auto StaticModelGroup::GetTexture(std::string const & textureName) const -> Texture * {
    return _textures.at(textureName).get();
}

auto StaticModelGroup::GetDefaultShaderProgram() -> ShaderProgram * {
    if (_defaultShaderProgram == nullptr) {
        _defaultShaderProgram = CreateShaderProgram("shader/default.vert", "shader/default.frag");
    }
    return _defaultShaderProgram;
}

auto StaticModelGroup::InitTransformData() -> void {
    glGenBuffers(1, &_transformUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, _transformUbo);
    glBufferData(GL_UNIFORM_BUFFER, Model::ShaderData::Size() * _shapes.size(), nullptr, GL_DYNAMIC_DRAW);
    // may use glBufferStorage() instead of glBufferData() on gl version 4.4+
    //glBufferStorage(GL_UNIFORM_BUFFER, _transformShaderDataSize * _shapes.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto StaticModelGroup::LoadTransformData() -> void {
    auto cache = vector<unsigned char>(Model::ShaderData::Size() * _models.size());
    auto offset = 0;
    for (auto & model : _models) {
        auto * p = reinterpret_cast<Movable::ShaderData *>(cache.data() + offset);
        *p = model->GetShaderData();
        model->SetUboOffset(offset);
        offset += Model::ShaderData::Size();
    }
    glBindBuffer(GL_UNIFORM_BUFFER, _transformUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, cache.size(), cache.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto StaticModelGroup::UseTransformData(Model const* model) -> void {
    glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Transform), _transformUbo, model->GetUboOffset(), Model::ShaderData::Size());
}

auto StaticModelGroup::LoadMaterialData() -> void {
    glGenBuffers(1, &_materialUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, _materialUbo);
    glBufferData(GL_UNIFORM_BUFFER, Material::ShaderData::Size() * _materials.size(), nullptr, GL_DYNAMIC_DRAW);
    auto * p = static_cast<unsigned char*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
    auto offset = 0;
    for (auto & m : _materials) {
        auto * material = m.second.get();
        material->SetUboOffset(offset);
        memcpy(p, &material->GetShaderData(), sizeof(Material::ShaderData));
        p += Material::ShaderData::Size();
        offset += Material::ShaderData::Size();
    }
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto StaticModelGroup::UseMaterialData(Material const* material) -> void {
    glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Material), _materialUbo, material->GetUboOffset(), Material::ShaderData::Size());
}
}
