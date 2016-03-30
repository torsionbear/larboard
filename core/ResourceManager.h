#pragma once

#include <vector>
#include <memory>

#include "Scene.h"
#include "StaticModelGroup.h"
#include "Mesh.h"
#include "Skybox.h"
#include "Bvh.h"
#include "Material.h"
#include "Model.h"
#include "TextureArray.h"
#include "Terrain.h"
#include "AmbientLight.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"

namespace core {

struct LightShaderData {
    enum {
        MaxDirectionalLightCount = 10,
        MaxPointLightCount = 50,
        MaxSpotLightCount = 50,
    };
    AmbientLight::ShaderData ambientLight;
    DirectionalLight::ShaderData directionalLights[MaxDirectionalLightCount];
    PointLight::ShaderData pointLights[MaxPointLightCount];
    SpotLight::ShaderData spotLights[MaxSpotLightCount];
    int directionalLightCount;
    int pointLightCount;
    int spotLightCount;
};

class ResourceManager {
public:
    static auto UpdateScene(ResourceManager * resourceManager, Scene const* scene) -> void;
private:
    struct VertexBuffer {
        openglUint _vao = 0;
        openglUint _vbo = 0;
        openglUint _veo = 0;
    };
public:
    ResourceManager::~ResourceManager();
public:
    auto LoadScene(Scene * scene) -> void;
    auto LoadStaticModelGroup(StaticModelGroup * staticModelGroup) -> void;
    auto LoadBvh(Bvh * bvh) -> void;
    auto LoadMeshes(std::vector<std::unique_ptr<Mesh<Vertex>>> const& meshes) -> void;
    auto LoadSkyBox(SkyBox * skyBox) -> void;

    auto LoadMaterials(std::vector<Material *> const& materials) -> void;
    auto LoadModels(std::vector<std::unique_ptr<Model>> const& models) -> void;
    auto LoadTexture(Texture * texture) -> void;
    auto LoadCubeMap(CubeMap * cubeMap) -> void;
    auto LoadTextureArray(TextureArray * textureArray) -> void;
    auto LoadAabbs(std::vector<Aabb *> aabbs) -> void;
    auto LoadTerrain(Terrain * terrain) -> void;
    auto LoadTerrainSpecialTiles(std::vector<Mesh<Vertex> *> terrainSpecialTiles) -> void;
    auto UpdateTerrain(Terrain * terrain, Camera * camera) -> void;
    auto InitCameraData(unsigned int size) -> void;
    auto UpdateCameraData(std::vector<std::unique_ptr<Camera>> const& cameras) -> void;
    auto UseCameraData(Camera const * camera) -> void;
    auto InitLightData(
        std::vector<std::unique_ptr<AmbientLight>> const& ambientLights,
        std::vector<std::unique_ptr<PointLight>> const& pointLights,
        std::vector<std::unique_ptr<DirectionalLight>> const& directionalLights,
        std::vector<std::unique_ptr<SpotLight>> const& spotLights) -> void;
private:
    auto LoadSkyBoxMesh(SkyBox * skyBox) -> void;
private:
    std::vector<VertexBuffer> _vertexBuffers;
    std::vector<openglUint> _instanceBuffers;
    std::vector<openglUint> _materialBuffers;
    std::vector<openglUint> _transformBuffers;
    std::vector<openglUint> _uniformBuffers;
    std::vector<openglUint> _textures;
    std::vector<std::unique_ptr<ShaderProgram>> _shaderPrograms;
    unsigned int _cameraUboIndex;
    //unsigned int _lightUboIndex;
};

}