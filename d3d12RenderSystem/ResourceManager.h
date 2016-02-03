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

#include "FrameManager.h"

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
    ResourceManager()
        : _swapChainRenderTargets(2) {
    }
    ~ResourceManager() {
        _fence.Sync();
    }
public:
    auto Init(ID3D12Device * device, ID3D12CommandQueue * commandQueue, IDXGIFactory1 * factory, UINT width, UINT height, HWND hwnd) -> void;
    auto FrameBegin() -> void;
    auto FrameEnd() -> void;
    auto LoadBegin() -> void;
    auto LoadEnd() -> void;
    auto LoadMeshes(std::vector<std::unique_ptr<core::Mesh>> const& meshes, unsigned int stride) -> void;
    auto GetMeshData(unsigned int index) -> MeshData const& {
        return _meshData[index];
    }
    auto GetRootSignature() -> ID3D12RootSignature * {
        return _rootSignature.Get();
    }
    auto GetSwapChainRenderTargets() -> SwapChainRenderTargets & {
        return _swapChainRenderTargets;
    }
    auto GetCommandList() -> ID3D12GraphicsCommandList * {
        return _commandList.Get();
    }
private:
    auto CreatePso(ID3D12RootSignature * rootSignature)->ComPtr<ID3D12PipelineState>;
    auto CreateRootSignature()->ComPtr<ID3D12RootSignature>;
    auto CreateCommandList(ID3D12PipelineState * pso, ID3D12CommandAllocator * allocator)->ComPtr<ID3D12GraphicsCommandList>;
private:
    ID3D12Device * _device;
    ComPtr<ID3D12Resource> _uploadHeap;
    ComPtr<ID3D12Resource> _uploadHeap2;
    ComPtr<ID3D12Resource> _vertexHeap;
    ComPtr<ID3D12Resource> _indexHeap;
    std::vector<MeshData> _meshData;

    Fence _fence;
    FrameManager _frameManager;
    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12RootSignature> _rootSignature;
    ID3D12CommandQueue * _commandQueue;
    SwapChainRenderTargets _swapChainRenderTargets;
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