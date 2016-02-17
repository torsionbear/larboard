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

    auto readRange = CD3DX12_RANGE(0, 0);		// We do not intend to read from this resource on the CPU.
    _uploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&_heapBegin));
    _heapEnd = _heapBegin + size;
    _begin = _heapBegin;
    _end = _heapBegin;
}

auto UploadHeap::UploadSubresources(ID3D12GraphicsCommandList * commandList, ID3D12Resource * dest, unsigned int first, unsigned int count, D3D12_SUBRESOURCE_DATA * subresources) -> void {
    auto memoryBlockSize = GetRequiredIntermediateSize(dest, first, count);
    auto const& memoryBlcok = AllocateMemoryBlock(memoryBlockSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
    UpdateSubresources(commandList, dest, _uploadHeap.Get(), memoryBlcok._offset, first, count, subresources);
}

auto UploadHeap::AllocateAndUploadDataBlock(ID3D12GraphicsCommandList * commandList, ID3D12Resource * dest, unsigned int size, unsigned int alignment, void * data) -> void {
    auto & memoryBlock = AllocateMemoryBlock(size, alignment);
    memcpy(_heapBegin + memoryBlock._offset, data, size);
    UploadMemoryBlock(commandList, memoryBlock, dest);
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
        memcpy(_heapBegin + memoryBlock._offset + dataBlock->offset, dataBlock->data, dataBlock->size);
    }
    return memoryBlock;
}

auto UploadHeap::UploadMemoryBlock(ID3D12GraphicsCommandList * commandList, MemoryBlock const & memoryBlock, ID3D12Resource * dest) -> void {
    commandList->CopyBufferRegion(dest, 0, _uploadHeap.Get(), memoryBlock._offset, memoryBlock._size);
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
        _memoryBlockQueue.push(MemoryBlock{ alignment, size, static_cast<unsigned int>(ptr - _heapBegin), _fencedCommandQueue->GetFenceValue() });
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
        _begin = _heapBegin + _memoryBlockQueue.front()._offset;
    }
}

}