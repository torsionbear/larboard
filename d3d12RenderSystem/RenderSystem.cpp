#include "RenderSystem.h"

#include <assert.h>
#include <array>

#include "d3dx12.h"
#include "Common.h"

using Microsoft::WRL::ComPtr;
using std::array;

namespace d3d12RenderSystem {

auto RenderSystem::Init(unsigned int width, unsigned int height) -> void {
    EnableDebugLayer();
    auto factory = CreateFactory();
    _device = CreateDevice(factory.Get());
    _fencedCommandQueue.Init(_device.Get());
    _renderWindow.Create(width, height, L"RenderWindow");
    _resourceManager.Init(_device.Get(), factory.Get(), &_fencedCommandQueue, width, height, _renderWindow.GetHwnd());
    _srvHeap = CreateSrvHeap(_device.Get(), 1);
    _renderer.Init(&_resourceManager, width, height);
}

auto RenderSystem::EnableDebugLayer() -> void {
#if defined(_DEBUG)
    // 0. enable d3d12 debug layer
    auto d3d12Debug = ComPtr<ID3D12Debug>{ nullptr };
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug)))) {
        d3d12Debug->EnableDebugLayer();
    }
#endif
}

auto RenderSystem::CreateFactory() -> ComPtr<IDXGIFactory1> {
    auto factory = ComPtr<IDXGIFactory1>{ nullptr };
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    return factory;
}

auto RenderSystem::CreateDevice(IDXGIFactory1 * factory) -> ComPtr<ID3D12Device> {
    ComPtr<ID3D12Device> device;

    auto adapter = ComPtr<IDXGIAdapter1>{ nullptr };
    for (auto i = 0u; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i) {
        auto description = DXGI_ADAPTER_DESC1{};
        adapter->GetDesc1(&description);
        //if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        //    continue;
        //}
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
            break;
        }
    }
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    return device;
}

auto RenderSystem::CreateSrvHeap(ID3D12Device * device, unsigned int descriptorCount) -> ComPtr<ID3D12DescriptorHeap> {
    auto ret = ComPtr<ID3D12DescriptorHeap>{ nullptr };
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = descriptorCount;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&ret)));
    return ret;
}

}