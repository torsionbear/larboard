#pragma once

#include <d3d12.h>

#include <wrl.h>

#include "d3dx12.h"
#include "Common.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

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

        _cpuHandle = _heap->GetCPUDescriptorHandleForHeapStart();
        _gpuHandle = _heap->GetGPUDescriptorHandleForHeapStart();
    }
    auto GetCpuHandle(unsigned int index) -> D3D12_CPU_DESCRIPTOR_HANDLE {
        return D3D12_CPU_DESCRIPTOR_HANDLE{ _cpuHandle.ptr + index * _incrementSize};
    }
    auto GetGpuHandle(unsigned int index) -> D3D12_GPU_DESCRIPTOR_HANDLE {
        return D3D12_GPU_DESCRIPTOR_HANDLE{ _gpuHandle.ptr + index * _incrementSize };
    }

private:
    CD3DX12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    unsigned int _size;
    unsigned int _incrementSize;
    ComPtr<ID3D12DescriptorHeap> _heap;
};

}