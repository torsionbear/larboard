#pragma once

#include <vector>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include "d3dx12.h"

#include "Common.h"
#include "FrameResource.h"
#include "SwapChainRenderTargets.h"
#include "Fence.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

class FrameManager {
public:
    ~FrameManager() {
        _fence->Sync();
    }
    auto Init(ID3D12Device * device, Fence * fence, unsigned int resourceCount) -> void {
        _frameResources = std::vector<FrameResource>(resourceCount);
        for (auto & frameResource : _frameResources) {
            frameResource.Init(device);
        }
        _fence = fence;
    }
    //auto GetCurrentFrameResource() -> FrameResource * {
    //    return &_frameResources[_frameCacheIndex];
    //}
    //auto FrameBegin() -> void {
    //    _frameCacheIndex = (_frameCacheIndex + 1) % _frameResources.size();
    //    _fence->Sync(_frameResources[_frameCacheIndex].GetFenceValue());
    //
    //    _frameResources[_frameCacheIndex].Reset();
    //}
    auto FrameEnd() -> void {
        _frameResources[_frameCacheIndex].SetFenceValue(_fence->Signal());
    }
    auto Sync(uint64 fenceValue) -> void {
        _fence->Sync(fenceValue);
    }
private:
    Fence * _fence;

    std::vector<FrameResource> _frameResources;
    unsigned int _frameCacheIndex = 0;
};

}