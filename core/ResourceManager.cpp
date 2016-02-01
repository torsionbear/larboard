#include "ResourceManager.h"

#include <vector>

#include <GL/glew.h>

using std::vector;
using std::unique_ptr;

namespace core {

ResourceManager::~ResourceManager() {
    if (!_uniformBuffers.empty()) {
        glDeleteBuffers(_uniformBuffers.size(), _uniformBuffers.data());
    }
}

auto ResourceManager::LoadMeshes(vector<unique_ptr<Mesh>> const& meshes) -> void {
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();

	glGenVertexArrays(1, &vertexBuffer._vao);
	glBindVertexArray(vertexBuffer._vao);

	glGenBuffers(1, &vertexBuffer._vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);

	glGenBuffers(1, &vertexBuffer._veo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);

	auto vertexData = vector<Vertex>();
	auto indexData = vector<unsigned int>();
	for (auto const& mesh : meshes) {
        mesh->SetRenderData(Mesh::RenderData{
            vertexBuffer._vao,
            indexData.size() * sizeof(unsigned int),
            static_cast<GLint>(vertexData.size()),
        });
		vertexData.insert(vertexData.end(), mesh->GetVertex().cbegin(), mesh->GetVertex().cend());
		indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
	}
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(Vector3f)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(Vector3f)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

auto ResourceManager::LoadSkyBoxMesh(SkyBox * skyBox) -> void {
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();

    auto const vertexData = vector<Vector3f>{
        Vector3f{ -1, 1, -1 },
        Vector3f{ -1, -1, -1 },
        Vector3f{ 1, -1, -1 },
        Vector3f{ 1, 1, -1 },
        Vector3f{ -1, 1, 1 },
        Vector3f{ -1, -1, 1 },
        Vector3f{ 1, -1, 1 },
        Vector3f{ 1, 1, 1 } };
    auto const indexData = vector<unsigned int>{ 0, 1, 2, 3, 0, 4, 5, 1, 0, 3, 7, 4, 2, 6, 7, 3, 1, 5, 6, 2, 7, 6, 5, 4 };

    glGenVertexArrays(1, &vertexBuffer._vao);
    glBindVertexArray(vertexBuffer._vao);
    skyBox->SetVao(vertexBuffer._vao);

    glGenBuffers(1, &vertexBuffer._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector3f), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer._veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

auto ResourceManager::LoadMaterials(vector<Material *> const& materials) -> void {
    _materialBuffers.emplace_back();
    auto & materialUbo = _materialBuffers.back();
    glGenBuffers(1, &materialUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, materialUbo);

    glBufferData(GL_UNIFORM_BUFFER, Material::ShaderData::Size() * materials.size(), nullptr, GL_DYNAMIC_DRAW);
    auto * p = static_cast<unsigned char*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
    auto offset = 0;
    for (auto & material : materials) {
        material->SetUbo(materialUbo);
        material->SetUboOffset(offset);
        memcpy(p, &material->GetShaderData(), sizeof(Material::ShaderData));
        p += Material::ShaderData::Size();
        offset += Material::ShaderData::Size();
    }
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto ResourceManager::LoadModels(vector<Model*> const & models) -> void {
    _transformBuffers.emplace_back();
    auto & transformUbo = _transformBuffers.back();
    glGenBuffers(1, &transformUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, transformUbo);

    auto cache = vector<unsigned char>(Model::ShaderData::Size() * models.size());
    auto offset = 0;
    for (auto & model : models) {
        model->SetUbo(transformUbo);
        model->SetUboOffset(offset);
        auto * p = reinterpret_cast<Movable::ShaderData *>(cache.data() + offset);
        *p = model->GetShaderData();
        offset += Model::ShaderData::Size();
    }
    //glBufferData(GL_UNIFORM_BUFFER, cache.size(), cache.data(), GL_DYNAMIC_DRAW);
    glBufferStorage(GL_UNIFORM_BUFFER, cache.size(), cache.data(), 0);
    // may use glBufferStorage() together with glBufferSubData() to update transform data for dynamic objects
    //glBufferStorage(GL_UNIFORM_BUFFER, cache.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
    //glBufferSubData(GL_UNIFORM_BUFFER, 0, cache.size(), cache.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto ResourceManager::LoadTexture(Texture * texture) -> void {
    _textures.emplace_back();
    auto & tex = _textures.back();
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    texture->SetTexture(tex);

    auto width = texture->GetWidth();
    auto height = texture->GetHeight();
    auto levelCount = static_cast<GLsizei>(floor(log2(std::max(width, height))) + 1);
    glTexStorage2D(GL_TEXTURE_2D, levelCount, GL_RGBA32F, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, texture->GetData().data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    // todo: wrap glGetError(), in release code, calling it only once per frame untill error happpens
    auto error = glGetError();
    assert(GL_NO_ERROR == error);
}

auto ResourceManager::LoadCubeMap(CubeMap * cubeMap) -> void {
    _textures.emplace_back();
    auto & texture = _textures.back();

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    cubeMap->SetTexture(texture);
    auto width = cubeMap->GetWidth();
    auto height = cubeMap->GetHeight();
    auto levelCount = static_cast<GLsizei>(floor(log2(std::max(width, height))) + 1);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, levelCount, GL_RGBA32F, width, height);
    auto const& data = cubeMap->GetData();
    for (auto i = 0u; i < 6; ++i) {
        auto target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
        glTexSubImage2D(target, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data[i].data());
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    if (GL_NO_ERROR != glGetError()) {
        MessageLogger::Log(MessageLogger::Error, "generating cube map failed.");
    }
}

auto ResourceManager::LoadTextureArray(TextureArray * textureArray) -> void {
    _textures.emplace_back();
    auto & texture = _textures.back();
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    textureArray->SetTexture(texture);

    auto width = textureArray->GetWidth();
    auto height = textureArray->GetHeight();
    auto levelCount = static_cast<GLsizei>(floor(log2(std::max(width, height))) + 1);
    auto const& data = textureArray->GetData();
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, levelCount, GL_RGBA32F, width, height, data.size());
    for (auto i = 0u; i < data.size(); ++i) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_FLOAT, data[i].data());
    }
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // repeat texture(suppose it's seamless)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    auto error = glGetError();
    assert(GL_NO_ERROR == error);
}

auto ResourceManager::LoadAabbs(std::vector<Aabb *> aabbs) -> void {
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();

    glGenVertexArrays(1, &vertexBuffer._vao);
    glBindVertexArray(vertexBuffer._vao);

    glGenBuffers(1, &vertexBuffer._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);

    glGenBuffers(1, &vertexBuffer._veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);

    auto vertexData = vector<Vector3f>{};
    auto indexData = vector<unsigned int>{};
    for (auto aabb : aabbs) {
        aabb->SetRenderData(Aabb::RenderData{
            vertexBuffer._vao,
            indexData.size() * sizeof(unsigned int),
            static_cast<GLint>(vertexData.size()),
        });
        auto const minVertex = aabb->GetMinVertex();
        auto const maxVertex = aabb->GetMaxVertex();
        vertexData.insert(vertexData.end(), {
            { minVertex(0), minVertex(1), minVertex(2) },
            { minVertex(0), maxVertex(1), minVertex(2) },
            { minVertex(0), maxVertex(1), maxVertex(2) },
            { minVertex(0), minVertex(1), maxVertex(2) },
            { maxVertex(0), minVertex(1), minVertex(2) },
            { maxVertex(0), maxVertex(1), minVertex(2) },
            { maxVertex(0), maxVertex(1), maxVertex(2) },
            { maxVertex(0), minVertex(1), maxVertex(2) },
        });
        auto const index = vector<unsigned int>{
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7,
        };
        indexData.insert(indexData.end(), index.begin(), index.end());
    }
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector3f), vertexData.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

auto ResourceManager::LoadTerrain(Terrain * terrain) -> void {
    auto const& shaderData = terrain->GetShaderData();
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();
    glGenVertexArrays(1, &vertexBuffer._vao);
    glBindVertexArray(vertexBuffer._vao);
    terrain->SetVao(vertexBuffer._vao);

    glGenBuffers(1, &vertexBuffer._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);
    auto tileSize = shaderData.tileSize;
    auto tileVertexData = std::array<Vector2f, 4>{ Vector2f{ 0, 0 }, Vector2f{ tileSize, 0 }, Vector2f{ tileSize, tileSize }, Vector2f{ 0, tileSize } };
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector2f), tileVertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vertexBuffer._veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);
    auto indexData = std::array<unsigned int, 6>{ 0, 1, 3, 1, 2, 3 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    // instance data
    _instanceBuffers.emplace_back();
    auto & vio = _instanceBuffers.back();
    glGenBuffers(1, &vio);
    glBindBuffer(GL_ARRAY_BUFFER, vio);
    terrain->SetVio(vio);
    auto sightDistance = shaderData.sightDistance;
    auto tileCountUpperBound = static_cast<int>(ceil(sightDistance / tileSize) * 2);
    tileCountUpperBound *= tileCountUpperBound;
    glBufferData(GL_ARRAY_BUFFER, tileCountUpperBound * sizeof(Vector3f), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // terrain ubo
    _uniformBuffers.emplace_back();
    auto & ubo = _uniformBuffers.back();
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, Terrain::ShaderData::Size(), &shaderData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Terrain), ubo);

    auto error = glGetError();
    assert(error == GL_NO_ERROR);
}

auto ResourceManager::LoadTerrainSpecialTiles(std::vector<Mesh*> terrainSpecialTiles) -> void {
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();
    glGenVertexArrays(1, &vertexBuffer._vao);
    glBindVertexArray(vertexBuffer._vao);

    glGenBuffers(1, &vertexBuffer._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);

    glGenBuffers(1, &vertexBuffer._veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);

    auto vertexData = vector<Vector2f>();
    auto indexData = vector<unsigned int>();
    for (auto mesh : terrainSpecialTiles) {
        mesh->SetRenderData(Mesh::RenderData{
            vertexBuffer._vao,
            indexData.size() * sizeof(unsigned int),
            static_cast<openglInt>(vertexData.size()),
        });
        for (auto const& vertex : mesh->GetVertex()) {
            vertexData.push_back(Vector2f{ vertex.coord(0), vertex.coord(1) });
        }
        indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
    }
    auto tileCoordData = vector<Vector2i>(vertexData.size(), Vector2i{ 0, 0 });
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector2f) + tileCoordData.size() * sizeof(Vector2i), vertexData.data(), GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(Vector2f), vertexData.data());
    glBufferSubData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector2f), tileCoordData.size() * sizeof(Vector2i), tileCoordData.data());

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(1, 2, GL_INT, sizeof(Vector2i), (GLvoid*)(vertexData.size() * sizeof(Vector2f)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    auto error = glGetError();
    assert(error == GL_NO_ERROR);
}

auto ResourceManager::UpdateTerrainTileCoordUbo(openglUint vio, std::vector<Vector3f> const & tileCoord) -> void {
    glBindBuffer(GL_ARRAY_BUFFER, vio);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vector3f) * tileCoord.size(), tileCoord.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

auto ResourceManager::InitCameraData(unsigned int size) -> void {
    _cameraUboIndex = _uniformBuffers.size();
    _uniformBuffers.emplace_back();
    auto & cameraUbo = _uniformBuffers.back();

    glGenBuffers(1, &cameraUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUbo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto ResourceManager::UpdateCameraData(vector<unique_ptr<Camera>> const& cameras) -> void {
    auto cache = vector<unsigned char>(Camera::ShaderData::Size() * cameras.size());
    auto offset = 0;
    for (auto & camera : cameras) {
        auto * p = reinterpret_cast<Camera::ShaderData *>(&cache[offset]);
        *p = camera->GetShaderData();
        camera->SetUboOffset(offset);
        offset += Camera::ShaderData::Size();
    }
    glBindBuffer(GL_UNIFORM_BUFFER, _uniformBuffers[_cameraUboIndex]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, cache.size(), cache.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto ResourceManager::UseCameraData(Camera const * camera) -> void {
    glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Camera), _uniformBuffers[_cameraUboIndex], camera->GetUboOffset(), sizeof(Camera::ShaderData));
}

auto ResourceManager::InitLightData(
    vector<unique_ptr<AmbientLight>> const& ambientLights,
    vector<unique_ptr<PointLight>> const& pointLights, 
    vector<unique_ptr<DirectionalLight>> const& directionalLights,
    vector<unique_ptr<SpotLight>> const& spotLights) -> void {
    auto data = LightShaderData{};

    data.ambientLight = ambientLights.front()->GetShaderData();

    data.directionalLightCount = directionalLights.size();
    assert(data.directionalLightCount <= LightShaderData::MaxDirectionalLightCount);
    for (auto i = 0; i < data.directionalLightCount; ++i) {
        data.directionalLights[i] = directionalLights[i]->GetShaderData();
    }

    data.pointLightCount = pointLights.size();
    assert(data.pointLightCount <= LightShaderData::MaxPointLightCount);
    for (auto i = 0; i < data.pointLightCount; ++i) {
        data.pointLights[i] = pointLights[i]->GetShaderData();
    }

    data.spotLightCount = spotLights.size();
    assert(data.spotLightCount <= LightShaderData::MaxSpotLightCount);
    for (auto i = 0; i < data.spotLightCount; ++i) {
        data.spotLights[i] = spotLights[i]->GetShaderData();
    }

    //_lightUboIndex = _uniformBuffers.size();
    _uniformBuffers.emplace_back();
    auto & lightUbo = _uniformBuffers.back();
    glGenBuffers(1, &lightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, lightUbo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &data, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Light), lightUbo);
}

}
