#include "Fence.h"

namespace d3d12RenderSystem {

Fence::~Fence() {
    CloseHandle(_fenceEvent);
}

auto Fence::Init(ID3D12Device * device, ID3D12CommandQueue * commandQueue) -> void {
    _commandQueue = commandQueue;
    ThrowIfFailed(device->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
    _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (_fenceEvent == nullptr) {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
    Sync();
}

auto Fence::Sync() -> void {
    Sync(Signal());
}

// fence is initialized with value 1, so fenceValue of 0 will be considerred done from begining
auto Fence::Sync(uint64 fenceValue) -> void {
    if (_fence->GetCompletedValue() < fenceValue) {
        ThrowIfFailed(_fence->SetEventOnCompletion(fenceValue, _fenceEvent));
        WaitForSingleObject(_fenceEvent, INFINITE);
    }
}

auto Fence::Signal() -> uint64 {
    ++_fenceValue;
    ThrowIfFailed(_commandQueue->Signal(_fence.Get(), _fenceValue));
    return _fenceValue;
}

auto Fence::GetCompleteFenceValue() -> uint64 {
    return _fence->GetCompletedValue();
}

}
