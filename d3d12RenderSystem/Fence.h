#pragma once

#include <wrl.h>
#include <d3d12.h>

#include "Common.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

class Fence {
public:
    ~Fence();
    auto Init(ID3D12Device * device, ID3D12CommandQueue * commandQueue) -> void;
    auto Sync() -> void;
    auto Sync(uint64 fenceValue) -> void;
    auto Signal() -> uint64;
    auto GetCompleteFenceValue() -> uint64;
private:
    ID3D12CommandQueue * _commandQueue;
    ComPtr<ID3D12Fence> _fence;
    HANDLE _fenceEvent;
    uint64 _fenceValue = 0;
};

}