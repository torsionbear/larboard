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
#include "core/Camera.h"
#include "SwapChainRenderTargets.h"
#include "FrameResource.h"
#include "FencedCommandQueue.h"
#include "UploadHeap.h"
#include "DescriptorHeap.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct MeshData {
    unsigned int vertexIndexBufferIndex;
    unsigned int indexCount;
    unsigned int indexOffset;
    int baseVertex;
};

struct CameraData {
    core::Matrix4x4f viewTransform;
    core::Matrix4x4f projectTransform;
    core::Matrix4x4f viewTransformInverse;
    core::Vector4f viewPosition;
};

struct VertexIndexBuffer {
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
};

struct ConstantBuffer {
    uint8 * _mappedDataPtr;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
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
    auto LoadCamera(core::Camera const& camera, ConstantBuffer const& constantBuffer, unsigned int offset) -> void;
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
    auto AllocDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int size) -> void;
    auto GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) -> ID3D12DescriptorHeap *;
    auto CreateDepthStencil(unsigned int width, unsigned int height, unsigned int index) -> D3D12_CPU_DESCRIPTOR_HANDLE;
    auto CreateConstantBuffer(unsigned int size, unsigned int index) -> ConstantBuffer;
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

    DescriptorHeap _dsvHeap;
    DescriptorHeap _cbvSrvHeap;
    std::vector<ComPtr<ID3D12Resource>> _depthStencils;
    std::vector<ComPtr<ID3D12Resource>> _constantBuffers;

    FrameResourceContainer<FrameResource, 2> _frameResourceContainer;
    SwapChainRenderTargets _swapChainRenderTargets;
    FencedCommandQueue * _fencedCommandQueue;

    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12RootSignature> _rootSignature;
};

}