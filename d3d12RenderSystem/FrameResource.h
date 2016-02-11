#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include "Common.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

template<typename FRAMERESOURCE, unsigned int SIZE>
class FrameResourceContainer {
public:
    template<typename ... ARGS>
    auto Init(ARGS&& ... args) -> void {
        for (auto & frameResource : _frameResources) {
            frameResource.Init(std::forward<ARGS>(args)...);
        }
    }
    auto GetCurrent() -> FRAMERESOURCE & {
        return _frameResources[_frameCacheIndex];
    }
    auto Switch() -> FRAMERESOURCE & {
        _frameCacheIndex = (_frameCacheIndex + 1) % _frameResources.size();
        return _frameResources[_frameCacheIndex];
    }
private:
    std::array<FRAMERESOURCE, SIZE> _frameResources;
    unsigned int _frameCacheIndex = 0;
};

class FrameResource {
public:
    auto Init(ID3D12Device* device) -> void {
        ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator)));
    }
    auto GetCommandAllocator() const -> ID3D12CommandAllocator * {
        return _commandAllocator.Get();
    }
    auto Reset() -> void {
        // Command list allocators can only be reset when the associated command lists have finished execution on the GPU
        _commandAllocator->Reset();
    }
    auto GetFenceValue() -> uint64 {
        return _fenceValue;
    }
    auto SetFenceValue(uint64 fenceValue) -> void {
        _fenceValue = fenceValue;
    }
private:
    uint64 _fenceValue = 0;
    ComPtr<ID3D12CommandAllocator> _commandAllocator;
};

}