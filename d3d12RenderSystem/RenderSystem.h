#pragma once

#include <memory>

#include <d3d12.h>
#include <dxgi1_4.h>

#include <wrl.h>

#include "RenderWindow.h"
#include "ResourceManager.h"
#include "Renderer.h"

#include "core/Shape.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct Vertex {
    float position[3];
    float color[4];
};

class RenderSystem {
public:
    auto GetResourceManager() -> ResourceManager & {
        return _resourceManager;
    }
    auto GetRenderer() -> Renderer & {
        return _renderer;
    }
    auto GetRenderWindow() -> RenderWindow & {
        return _renderWindow;
    }
    auto Init(unsigned int width, unsigned int height) -> void;
private:
    auto static EnableDebugLayer() -> void;
    auto static CreateFactory() -> ComPtr<IDXGIFactory1>;
    auto static CreateDevice(IDXGIFactory1 * factory) -> ComPtr<ID3D12Device>;
    auto static CreateCommandQueue(ID3D12Device * device) -> ComPtr<ID3D12CommandQueue>;
    auto static CreateSrvHeap(ID3D12Device * device, unsigned int descriptorCount) -> ComPtr<ID3D12DescriptorHeap>;

private:
    ComPtr<ID3D12Device> _device;
    ComPtr<ID3D12CommandQueue> _commandQueue;

    ComPtr<ID3D12DescriptorHeap> _srvHeap;
    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12Resource> _uploadHeap;

    ResourceManager _resourceManager;
    Renderer _renderer;
    RenderWindow _renderWindow;
};

}