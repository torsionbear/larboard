#pragma once

#include <memory>

#include <d3d12.h>
#include <dxgi1_4.h>

#include <wrl.h>

#include "RenderWindow.h"
#include "FrameManager.h"
#include "ResourceManager.h"
#include "Fence.h"
#include "SwapChainRenderTargets.h"

#include "core/Shape.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct Vertex {
    float position[3];
    float color[4];
};

class RenderSystem {
public:
    RenderSystem(RenderWindow * renderWindow)
        : _renderWindow(renderWindow)
        , _swapChainRenderTargets(2) {
        _viewport.Width = static_cast<float>(renderWindow->GetWidth());
        _viewport.Height = static_cast<float>(renderWindow->GetHeight());
        _viewport.MaxDepth = 1.0f;
        _scissorRect.right = static_cast<LONG>(renderWindow->GetWidth());
        _scissorRect.bottom = static_cast<LONG>(renderWindow->GetHeight());
    }
    ~RenderSystem() {
        _fence.Sync();
    }
public:
    auto Render(core::Shape const* shape) -> void;
    auto Init() -> void;
    auto Update() -> void;
    auto RenderBegin() -> void;
    auto RenderEnd() -> void;
    auto LoadBegin() -> void;
    auto LoadMeshes(std::vector<std::unique_ptr<core::Mesh>> const& meshes) -> void;
    auto LoadEnd() -> void;
private:
    auto static EnableDebugLayer() -> void;
    auto static CreateFactory() -> ComPtr<IDXGIFactory1>;
    auto static CreateDevice(IDXGIFactory1 * factory) -> ComPtr<ID3D12Device>;
    auto static CreateCommandQueue(ID3D12Device * device) -> ComPtr<ID3D12CommandQueue>;
    auto static CreateSrvHeap(ID3D12Device * device, unsigned int descriptorCount) -> ComPtr<ID3D12DescriptorHeap>;
    auto static CreateRootSignature(ID3D12Device * device) -> ComPtr<ID3D12RootSignature>;
    auto static CreatePso(ID3D12Device * device, ID3D12RootSignature * rootSignature) -> ComPtr<ID3D12PipelineState>;
    auto static CreateCommandList(ID3D12Device * device, ID3D12PipelineState * pso, ID3D12CommandAllocator * allocator) -> ComPtr<ID3D12GraphicsCommandList>;

private:
    static unsigned int const _frameCount = 2;

    Fence _fence;
    SwapChainRenderTargets _swapChainRenderTargets;
    ResourceManager _resourceManager;

    ComPtr<ID3D12Device> _device;
    ComPtr<ID3D12CommandQueue> _commandQueue;

    ComPtr<ID3D12DescriptorHeap> _srvHeap;
    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12RootSignature> _rootSignature;
    //ComPtr<ID3D12Resource> _vertexBuffer;
    //D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
    ComPtr<ID3D12Resource> _uploadHeap;
    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

    RenderWindow * _renderWindow;
    FrameManager _frameManager;
};

}