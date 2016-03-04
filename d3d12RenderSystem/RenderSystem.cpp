#include "RenderSystem.h"

#include <assert.h>
#include <array>

#include "d3dx12.h"
#include "Common.h"
#include "SsaoRenderer.h"

using Microsoft::WRL::ComPtr;
using std::array;
using std::make_unique;

namespace d3d12RenderSystem {

RenderSystem::~RenderSystem() {
    _resourceManager.reset();
    _renderer.reset();
#if defined(_DEBUG)
    //check if any d3d object is leaked
    _dxgiDebug1->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif
}

auto RenderSystem::Init(unsigned int width, unsigned int height) -> void {
    EnableDebugLayer();
    auto factory = CreateFactory();
    _renderWindow.Create(width, height, L"RenderWindow");
    _resourceManager = make_unique<ResourceManager>(factory.Get(), width, height, _renderWindow.GetHwnd());
    _renderer = make_unique<SsaoRenderer>(_resourceManager.get(), width, height);
}

auto RenderSystem::EnableDebugLayer() -> void {
#if defined(_DEBUG)
    auto d3d12Debug = ComPtr<ID3D12Debug>{ nullptr };
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug)))) {
        d3d12Debug->EnableDebugLayer();
    }
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&_dxgiDebug1));
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

}