#pragma once

#include <queue>

#include <wrl.h>

#include <d3d12.h>

#include "Common.h"
#include "FencedCommandQueue.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct DataBlock {
    unsigned int offset;
    unsigned int size;
    unsigned int alignment;
    void * data;
};

struct MemoryBlock {
    unsigned int alignment;
    unsigned int _size;
    unsigned int _offset;
    uint64 _fenceValue;
};

class UploadHeap {
public:
    auto Init(unsigned int size, ID3D12Device * device, FencedCommandQueue * fencedCommandQueue) -> void;
    auto UploadSubresources(ID3D12GraphicsCommandList * commandList, ID3D12Resource * dest, unsigned int first, unsigned int count, D3D12_SUBRESOURCE_DATA * subresources) -> void;
    auto AllocateAndUploadDataBlock(ID3D12GraphicsCommandList * commandList, ID3D12Resource * dest, unsigned int size, unsigned int alignment, void * data) -> void;
    auto AllocateDataBlocks(DataBlock * dataBlocks, unsigned int count) -> MemoryBlock const&;
    auto UploadMemoryBlock(ID3D12GraphicsCommandList * commandList, MemoryBlock const& memoryBlock, ID3D12Resource * dest) -> void;
private:
    //auto Alloc(unsigned int size, uint64 fenceValue) -> MemoryBlock &;
    auto TryAllocate(unsigned int size, unsigned int alignment) -> uint8 *;
    auto AllocateMemoryBlock(unsigned int size, unsigned int alignment)->MemoryBlock &;
    auto ReleaseCompletedMemoryBlock() -> void;

private:
    std::queue<MemoryBlock> _memoryBlockQueue;
    ComPtr<ID3D12Resource> _uploadHeap;
    FencedCommandQueue * _fencedCommandQueue;

    uint8 * _heapBegin = nullptr;
    uint8 * _heapEnd = nullptr;
    uint8 * _begin = nullptr;
    uint8 * _end = nullptr;
};

}