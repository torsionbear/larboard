#include "UploadHeap.h"

#include "d3dx12.h"

namespace d3d12RenderSystem {

auto UploadHeap::Init(unsigned int size, ID3D12Device * device, FencedCommandQueue * fencedCommandQueue) -> void {
    _fencedCommandQueue = fencedCommandQueue;
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(size),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_uploadHeap)));

    _uploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&_heapBegin));
    _heapEnd = _heapBegin + size;
    _begin = _heapBegin;
    _end = _heapBegin;
}

auto UploadHeap::AllocateDataBlocks(DataBlock * dataBlocks, unsigned int count) -> MemoryBlock const& {
    // 1. compute alignments' lcd
    // alignments' lowest common denominator should be the greatest one.
    auto alignmentLcd = 0u;
    for (auto i = 0u; i < count; ++i) {
        auto const alignment = (dataBlocks + i)->alignment;
        if (alignment > alignmentLcd) {
            alignmentLcd = alignment;
        }
    }
    // 2. compute total size
    auto totalSize = 0u;
    for (auto i = 0u; i < count; ++i) {
        auto dataBlock = dataBlocks + i;
        auto dataBegin = Align(totalSize, dataBlock->alignment);
        dataBlock->offset = dataBegin;
        totalSize = dataBegin + dataBlock->size;
    }
    // 3. allocate memory block
    auto & memoryBlock = AllocateMemoryBlock(totalSize, alignmentLcd);
    // 4. copy data
    for (auto i = 0u; i < count; ++i) {
        auto dataBlock = (dataBlocks + i);
        memcpy(memoryBlock._ptr + dataBlock->offset, dataBlock->data, dataBlock->size);
    }
    return memoryBlock;
}

auto UploadHeap::UploadDataBlocks(ID3D12GraphicsCommandList * commandList, MemoryBlock const & memoryBlock, ID3D12Resource * dest) -> void {
    commandList->CopyBufferRegion(dest, 0, _uploadHeap.Get(), memoryBlock._ptr - _heapBegin, memoryBlock._size);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
}

auto UploadHeap::UploadDataBlocks(ID3D12GraphicsCommandList * commandList, DataBlock * dataBlocks, unsigned int count, ID3D12Resource * dest) -> void {
    // 1. compute alignments' lcd
    // alignments' lowest common denominator should be the greatest one.
    auto alignmentLcd = 0u;
    for (auto i = 0u; i < count; +i) {
        auto const alignment = (dataBlocks + i)->alignment;
        if (alignment > alignmentLcd) {
            alignmentLcd = alignment;
        }
    }
    // 2. compute total size
    auto totalSize = 0u;
    for (auto i = 0u; i < count; ++i) {
        auto dataBlock = dataBlocks + i;
        auto dataBegin = Align(totalSize, dataBlock->alignment);
        dataBlock->offset = dataBegin;
        totalSize = dataBegin + dataBlock->size;
    }
    // 3. allocate memory block
    auto & memoryBlock = AllocateMemoryBlock(totalSize, alignmentLcd);
    // 4. copy data
    for (auto i = 0u; i < count; ++i) {
        auto dataBlock = (dataBlocks + i);
        memcpy(memoryBlock._ptr + dataBlock->offset, dataBlock->data, dataBlock->size);
    }
    // 5. upload
    commandList->CopyBufferRegion(dest, 0, _uploadHeap.Get(), memoryBlock._ptr - _heapBegin, memoryBlock._size);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
}

//auto UploadHeap::Alloc(unsigned int size, uint64 fenceValue) -> MemoryBlock & {
//    auto cur = _end;
//    if ((_end + size < _begin) || (_end >= _begin && _end + size <= _heapEnd)) {
//        _memoryBlockQueue.push(MemoryBlock{_end, size, fenceValue});
//        _end = cur + size;
//    } else if (_end >= _begin && _heapBegin + size < _begin) {
//        _memoryBlockQueue.push(MemoryBlock{ _heapBegin, size, fenceValue });
//        _end = _heapBegin + size;
//    } else {
//        throw;
//    }
//    return _memoryBlockQueue.back();
//}

auto UploadHeap::TryAllocate(unsigned int size, unsigned int alignment) -> uint8* {
    auto cur = Align(_end, alignment);
    if ((cur + size < _begin) || (cur >= _begin && cur + size <= _heapEnd) || (cur >= _begin && (cur = Align(_heapBegin, alignment)) + size < _begin)) {
        return cur;
    }
    return nullptr;
}

auto UploadHeap::AllocateMemoryBlock(unsigned int size, unsigned int alignment) -> MemoryBlock & {
    auto ptr = TryAllocate(size, alignment);
    if (ptr == nullptr) {
        ReleaseCompletedMemoryBlock();
        ptr = TryAllocate(size, alignment);
    }
    if (ptr != nullptr) {
        _end = ptr + size;
        _memoryBlockQueue.push(MemoryBlock{ alignment, size, ptr, _fencedCommandQueue->GetFenceValue() });
    } else {
        throw;
    }
    return _memoryBlockQueue.back();
}

auto UploadHeap::ReleaseCompletedMemoryBlock() -> void {
    auto const completedValue = _fencedCommandQueue->GetCompletedValue();
    while (!_memoryBlockQueue.empty() && _memoryBlockQueue.front()._fenceValue <= completedValue) {
        _memoryBlockQueue.pop();
    }
    if (_memoryBlockQueue.empty()) {
        _begin = _heapBegin;
        _end = _heapBegin;
    } else {
        _begin = _memoryBlockQueue.front()._ptr;
    }
}

}