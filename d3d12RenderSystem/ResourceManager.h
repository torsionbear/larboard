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
#include "core/Model.h"
#include "core/Camera.h"
#include "SwapChainRenderTargets.h"
#include "FrameResource.h"
#include "FencedCommandQueue.h"
#include "UploadHeap.h"
#include "DescriptorHeap.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct MeshDataInfo {
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
    unsigned int indexCount;
    unsigned int indexOffset;
    int baseVertex;
};
// constant buffer data members must be 4*32 bytes aligned, constant buffer itself must be D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT(256) bytes aligned.
struct CameraData {
    core::Matrix4x4f viewTransform;
    core::Matrix4x4f projectTransform;
    core::Matrix4x4f viewTransformInverse;
    core::Vector4f viewPosition;
    core::Matrix3x4f _pad;
};

struct TransformData {
    core::Matrix4x4f worldTransform;
    core::Matrix4x4f normalTransform;
    core::Matrix4x4f _pad1;
    core::Matrix4x4f _pad2;
};

//struct VertexIndexBuffer {
//    D3D12_VERTEX_BUFFER_VIEW vbv;
//    D3D12_INDEX_BUFFER_VIEW ibv;
//};

struct PersistentMappedBuffer{
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
    auto LoadBegin(unsigned int depthStencilCount, unsigned int cameraCount, unsigned int meshCount, unsigned int modelCount) -> void;
    auto LoadEnd() -> void;
    auto LoadMeshes(core::Mesh ** meshes, unsigned int count, unsigned int stride) -> void;
    auto LoadModels(core::Model ** models, unsigned int count) -> void;
    auto LoadCamera(core::Camera * camera, unsigned int count) -> void;
    auto UpdateCamera(core::Camera const& camera) -> void;
    auto CreateDepthStencil(unsigned int width, unsigned int height) -> void;
    auto GetMeshDataInfo(unsigned int index) -> MeshDataInfo const& {
        return _meshDataInfos[index];
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
    auto GetCbvSrvDescriptorHeap() const -> DescriptorHeap const& {
        return _cbvSrvHeap;
    }
    auto GetDsvDescriptorHeap() const -> DescriptorHeap const& {
        return _dsvHeap;
    }
    auto AllocCbvSrvDescriptorHeap(unsigned int size) -> void {
        _cbvSrvHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    }
    auto AllocDsvDescriptorHeap(unsigned int size) -> void {
        _dsvHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, size, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    }
    auto GetCameraBufferInfo(unsigned int index) -> BufferInfo const& {
        return _cameraBufferInfos[index];
    }
    auto GetTransformBufferInfo(unsigned int index) -> BufferInfo const& {
        return _transformBufferInfos[index];
    }
    auto GetDepthStencilBufferInfo(unsigned int index) -> BufferInfo const& {
        return _depthStencilBufferInfos[index];
    }
private:
    auto CreatePso(ID3D12RootSignature * rootSignature)->ComPtr<ID3D12PipelineState>;
    auto CreateRootSignature()->ComPtr<ID3D12RootSignature>;
    auto CreateCommandList(ID3D12PipelineState * pso, ID3D12CommandAllocator * allocator)->ComPtr<ID3D12GraphicsCommandList>;
private:
    ID3D12Device * _device;
    UploadHeap _uploadHeap;

    DescriptorHeap _dsvHeap;
    DescriptorHeap _cbvSrvHeap;
    std::vector<MeshDataInfo> _meshDataInfos;
    std::vector<BufferInfo> _cameraBufferInfos;
    std::vector<BufferInfo> _transformBufferInfos;
    std::vector<BufferInfo> _depthStencilBufferInfos;

    std::vector<ComPtr<ID3D12Resource>> _vertexBuffers;
    std::vector<ComPtr<ID3D12Resource>> _indexBuffers;
    std::vector<ComPtr<ID3D12Resource>> _uploadBuffers;
    std::vector<ComPtr<ID3D12Resource>> _defaultBuffers;

    FrameResourceContainer<FrameResource, 2> _frameResourceContainer;
    SwapChainRenderTargets _swapChainRenderTargets;
    FencedCommandQueue * _fencedCommandQueue;

    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12RootSignature> _rootSignature;
};

}