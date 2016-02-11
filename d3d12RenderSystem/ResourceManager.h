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
#include "SwapChainRenderTargets.h"
#include "FrameResource.h"
#include "FencedCommandQueue.h"
#include "UploadHeap.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct MeshData {
    unsigned int vertexIndexBufferIndex;
    unsigned int indexCount;
    unsigned int indexOffset;
    int baseVertex;
};

struct VertexIndexBuffer {
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
};

class ResourceManager {
public:
    ResourceManager() {
    }
public:
    auto Init(ID3D12Device * device, IDXGIFactory1 * factory, FencedCommandQueue * fencedCommandQueue, unsigned int width, unsigned int height, HWND hwnd) -> void;
    auto PrepareResource() -> void;
    auto LoadEnd() -> void;
    auto LoadMeshes(std::vector<std::unique_ptr<core::Mesh>> const& meshes, unsigned int stride) -> void;
    auto GetMeshData(unsigned int index) -> MeshData const& {
        return _meshData[index];
    }
    auto GetVertexIndexBuffer(unsigned int index) -> VertexIndexBuffer const& {
        return _vertexIndexBuffer[index];
    }
    auto GetRootSignature() -> ID3D12RootSignature * {
        return _rootSignature.Get();
    }
    auto GetCommandList() -> ID3D12GraphicsCommandList * {
        return _commandList.Get();
    }
    auto GetFrameResource() -> FrameResource& {
        return _frameResourceContainer.GetCurrent();
    }
    auto GetSwapChainRenderTargets() -> SwapChainRenderTargets & {
        return _swapChainRenderTargets;
    }
    auto GetFencedCommandQueue() -> FencedCommandQueue * {
        return _fencedCommandQueue;
    }
private:
    auto CreatePso(ID3D12RootSignature * rootSignature)->ComPtr<ID3D12PipelineState>;
    auto CreateRootSignature()->ComPtr<ID3D12RootSignature>;
    auto CreateCommandList(ID3D12PipelineState * pso, ID3D12CommandAllocator * allocator)->ComPtr<ID3D12GraphicsCommandList>;
private:
    ID3D12Device * _device;
    UploadHeap _uploadHeap;
    ComPtr<ID3D12Resource> _vertexIndexHeap;
    std::vector<MeshData> _meshData;
    std::vector<VertexIndexBuffer> _vertexIndexBuffer;

    FrameResourceContainer<FrameResource, 2> _frameResourceContainer;
    SwapChainRenderTargets _swapChainRenderTargets;
    FencedCommandQueue * _fencedCommandQueue;

    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12RootSignature> _rootSignature;
};

}