#pragma once

#include <vector>
#include <memory>

//#include "Mesh.h"
//#include "Skybox.h"
//#include "Bvh.h"
//#include "Material.h"
//#include "Model.h"
//#include "TextureArray.h"
//#include "Terrain.h"

#include <d3d12.h>
//#include <dxgi1_4.h>

#include <wrl.h>

#include "core/Mesh.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct MeshData {
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
    unsigned int indexCount;
    unsigned int indexOffset;
    int baseVertex;
};

class ResourceManager {
public:
    auto Init(ID3D12Device * device) -> void {
        _device = device;
    }
    auto LoadMeshes(ID3D12GraphicsCommandList * commandList, std::vector<std::unique_ptr<core::Mesh>> const& meshes, unsigned int stride) -> void;
    auto GetMeshData(unsigned int index) -> MeshData const& {
        return _meshData[index];
    }
private:
    ID3D12Device * _device;
    ComPtr<ID3D12Resource> _uploadHeap;
    ComPtr<ID3D12Resource> _uploadHeap2;
    ComPtr<ID3D12Resource> _vertexHeap;
    ComPtr<ID3D12Resource> _indexHeap;
    std::vector<MeshData> _meshData;
};
/*
class ResourceManager {
private:
    struct VertexBuffer {
        openglUint _vao = 0;
        openglUint _vbo = 0;
        openglUint _veo = 0;
    };
public:
    auto LoadMeshes(std::vector<std::unique_ptr<Mesh>> const& meshes) -> void;
    auto LoadSkyBoxMesh(SkyBox * skyBox) -> void;

    auto LoadMaterials(std::vector<Material *> const& materials) -> void;
    auto LoadModels(std::vector<Model *> const& models) -> void;
    auto LoadTexture(Texture * texture) -> void;
    auto LoadCubeMap(CubeMap * cubeMap) -> void;
    auto LoadTextureArray(TextureArray * textureArray) -> void;
    auto LoadAabbs(std::vector<Aabb *> aabbs) -> void;
    auto LoadTerrain(Terrain * terrain) -> void;
    auto LoadTerrainSpecialTiles(std::vector<Mesh *> terrainSpecialTiles) -> void;
    auto UpdateTerrainTileCoordUbo(openglUint vio, std::vector<Vector3f> const& tileCoord) -> void;
private:
	std::vector<VertexBuffer> _vertexBuffers;
    std::vector<openglUint> _instanceBuffers;
    std::vector<openglUint> _materialBuffers;
    std::vector<openglUint> _transformBuffers;
    std::vector<openglUint> _uniformBuffers;
    std::vector<openglUint> _textures;
    std::vector<std::unique_ptr<ShaderProgram>> _shaderPrograms;
};
*/
}