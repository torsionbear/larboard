#include "SwapChainRenderTargets.h"

#include "d3dx12.h"

#include "Common.h"

namespace d3d12RenderSystem {

auto SwapChainRenderTargets::Init(IDXGIFactory1 * factory, ID3D12Device * device, ID3D12CommandQueue * commandQueue, UINT width, UINT height, HWND hwnd) -> void {
    // create swap chain
    auto swapChainDesc = DXGI_SWAP_CHAIN_DESC{};
    swapChainDesc.BufferCount = _backBufferCount;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    auto swapChain = ComPtr<IDXGISwapChain>{};
    ThrowIfFailed(factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain));
    ThrowIfFailed(swapChain.As(&_swapChain));

    // create rtv heap
    auto descriptorHeapDesc = D3D12_DESCRIPTOR_HEAP_DESC{};
    descriptorHeapDesc.NumDescriptors = _backBufferCount;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_rtvHeap)));

    // create rtv and rt resources
    auto rtvHeapStart = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
    auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    _renderTargets.resize(_backBufferCount);

    for (auto i = 0u; i < _renderTargets.size(); ++i) {
        auto & rt = _renderTargets[i];
        rt._descriptor = { rtvHeapStart.ptr + i * rtvDescriptorSize };
        ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&rt._resource)));
        device->CreateRenderTargetView(rt._resource.Get(), nullptr, rt._descriptor);
        rt._renderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(rt._resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        rt._presentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(rt._resource.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    _currentRenderTargetIndex = _swapChain->GetCurrentBackBufferIndex();
}

}