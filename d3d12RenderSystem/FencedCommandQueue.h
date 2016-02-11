#pragma once

#include <vector>

#include <wrl.h>
#include <d3d12.h>

#include "Common.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

class FencedCommandQueue {
public:
    ~FencedCommandQueue() {
        CloseHandle(_fenceEvent);
    }
    auto Init(ID3D12Device * device) -> void {
        auto commandQueueDesc = D3D12_COMMAND_QUEUE_DESC{};
        //commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        //commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ThrowIfFailed(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&_commandQueue)));

        ThrowIfFailed(device->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
        _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (_fenceEvent == nullptr) {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
        Signal(); // initial sync, so fenceValue of 0 will be considerred done from begining
    }
    auto ExecuteCommandList(ID3D12GraphicsCommandList * begin, unsigned int count) -> void {
        auto commandLists = std::vector<ID3D12CommandList *>{};
        for (auto i = 0u; i < count; ++i) {
            commandLists.push_back(begin + i);
        }
        _commandQueue->ExecuteCommandLists(count, commandLists.data());
        Signal();
    }
    auto GetCompletedValue() -> uint64 {
        return _fence->GetCompletedValue();
    }
    auto SyncLatest() -> void {
        Sync(_fenceValue - 1);
    }
    auto Sync(uint64 fenceValue) -> void {
        if (_fence->GetCompletedValue() < fenceValue) {
            ThrowIfFailed(_fence->SetEventOnCompletion(fenceValue, _fenceEvent));
            WaitForSingleObject(_fenceEvent, INFINITE);
        }
    }
    auto GetCommandQueue() -> ID3D12CommandQueue * {
        return _commandQueue.Get();
    }
    auto GetFenceValue() -> uint64 {
        return _fenceValue;
    }
private:
    auto Signal() -> void {
        ThrowIfFailed(_commandQueue->Signal(_fence.Get(), _fenceValue));
        ++_fenceValue;
    }
private:
    ComPtr<ID3D12CommandQueue> _commandQueue;
    ComPtr<ID3D12Fence> _fence;
    HANDLE _fenceEvent;
    uint64 _fenceValue = 0;
};

}