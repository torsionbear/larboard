#pragma once

#include <queue>

#include <wrl.h>

#include <d3d12.h>

#include "core/Primitive.h"
#include "Common.h"
#include "FencedCommandQueue.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct DataBlock {
    core::uint32 offset;
    core::uint32 size;
    core::uint32 alignment;
    void * data;
};

struct MemoryBlock {
    core::uint32 alignment;
    core::uint32 _size;
    core::uint32 _offset;
    core::uint64 _fenceValue;
};

class UploadHeap {
public:
    auto Init(uint64 size, ID3D12Device * device, FencedCommandQueue * fencedCommandQueue) -> void;
    auto UploadSubresources(ID3D12GraphicsCommandList * commandList, ID3D12Resource * dest, unsigned int first, unsigned int count, D3D12_SUBRESOURCE_DATA * subresources) -> void;
    auto AllocateAndUploadDataBlock(ID3D12GraphicsCommandList * commandList, ID3D12Resource * dest, unsigned int size, unsigned int alignment, void * data) -> void;
    auto AllocateDataBlocks(DataBlock * dataBlocks, unsigned int count) -> MemoryBlock const&;
    auto UploadMemoryBlock(ID3D12GraphicsCommandList * commandList, MemoryBlock const& memoryBlock, ID3D12Resource * dest) -> void;
private:
    auto TryAllocate(unsigned int size, unsigned int alignment) -> uint8 *;
    auto AllocateMemoryBlock(unsigned int size, unsigned int alignment)->MemoryBlock &;
    auto ReleaseCompletedMemoryBlock() -> void;

private:
    std::queue<MemoryBlock> _memoryBlockQueue;
    ComPtr<ID3D12Resource> _uploadHeap;
    FencedCommandQueue * _fencedCommandQueue;

    core::uint8 * _heapBegin = nullptr;
    core::uint8 * _heapEnd = nullptr;
    core::uint8 * _begin = nullptr;
    core::uint8 * _end = nullptr;
};

}