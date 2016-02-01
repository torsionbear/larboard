#pragma once

#include <vector>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include "Common.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

class SwapChainRenderTargets {
    struct RenderTarget {
        ComPtr<ID3D12Resource> _resource;
        D3D12_CPU_DESCRIPTOR_HANDLE _descriptor;
        D3D12_RESOURCE_BARRIER _renderTargetToPresent;
        D3D12_RESOURCE_BARRIER _presentToRenderTarget;
    };
public:
    SwapChainRenderTargets(unsigned int backBufferCount)
        : _backBufferCount(backBufferCount) {
    }
    auto Init(IDXGIFactory1 * factory, ID3D12Device * device, ID3D12CommandQueue * commandQueue, UINT width, UINT height, HWND hwnd) -> void;
    auto GetCurrentRtv() const -> D3D12_CPU_DESCRIPTOR_HANDLE {
        return _renderTargets[_currentRenderTargetIndex]._descriptor;
    }
    auto GetCurrentBackBuffer() const -> ID3D12Resource * {
        return _renderTargets[_currentRenderTargetIndex]._resource.Get();
    }
    auto Present() -> void {
        ThrowIfFailed(_swapChain->Present(1, 0));
        _currentRenderTargetIndex = _swapChain->GetCurrentBackBufferIndex();
    }
    auto GetBarrierToPresent() const -> D3D12_RESOURCE_BARRIER const* {
        return &_renderTargets[_currentRenderTargetIndex]._renderTargetToPresent;
    }
    auto GetBarrierToRenderTarget() const -> D3D12_RESOURCE_BARRIER const* {
        return &_renderTargets[_currentRenderTargetIndex]._presentToRenderTarget;
    }
private:
    unsigned int const _backBufferCount;
    ComPtr<IDXGISwapChain3> _swapChain;
    ComPtr<ID3D12DescriptorHeap> _rtvHeap;
    std::vector<RenderTarget> _renderTargets;
    UINT _currentRenderTargetIndex;
};

}