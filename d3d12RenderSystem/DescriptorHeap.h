#pragma once

#include <d3d12.h>

#include <wrl.h>

#include "d3dx12.h"
#include "Common.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct DescriptorInfo {
    uint8 * _mappedDataPtr;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
};

class DescriptorHeap {
public:
    auto Init(ID3D12Device * device, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int size, D3D12_DESCRIPTOR_HEAP_FLAGS flags) -> void {
        _size = size;
        _incrementSize = device->GetDescriptorHandleIncrementSize(type);

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = size;
        desc.Type = type;
        desc.Flags = flags;
        ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_heap)));

        _gpuHandle = _heap->GetGPUDescriptorHandleForHeapStart();
        _cpuHandle = _heap->GetCPUDescriptorHandleForHeapStart();
    }
    auto GetDescriptorInfo() -> DescriptorInfo {
        assert(_end <= _size);
        auto ret = DescriptorInfo{ nullptr, GetGpuHandle(_end), GetCpuHandle(_end) };
        ++_end;
        return ret;
    }
    auto GetHeap() const -> ID3D12DescriptorHeap * {
        return _heap.Get();
    }
private:
    auto GetGpuHandle(unsigned int index) const -> D3D12_GPU_DESCRIPTOR_HANDLE {
        return D3D12_GPU_DESCRIPTOR_HANDLE{ _gpuHandle.ptr + index * _incrementSize };
    }
    auto GetCpuHandle(unsigned int index) const -> D3D12_CPU_DESCRIPTOR_HANDLE {
        return D3D12_CPU_DESCRIPTOR_HANDLE{ _cpuHandle.ptr + index * _incrementSize };
    }
private:
    CD3DX12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    unsigned int _size;
    unsigned int _end = 0u;
    unsigned int _incrementSize;
    ComPtr<ID3D12DescriptorHeap> _heap;
};

}