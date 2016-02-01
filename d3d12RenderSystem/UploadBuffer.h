#pragma once

#include <queue>

#include <d3d12.h>
#include <dxgi1_4.h>

#include <wrl.h>

#include "d3dx12.h"
#include "Common.h"
#include "Fence.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

class RingBuffer {
private:
    struct FrameResourceOffset {
        UINT frameIndex;
        UINT8* pResourceOffset;
        uint8 fenceValue;
    };

    void DrawFrame() {
        float vertices[] = ...;
        UINT verticesOffset = 0;
        ThrowIfFailed(
            SetDataToUploadHeap(
                vertices, sizeof(float), sizeof(vertices) / sizeof(float),
                4, // Max alignment requirement for vertex data is 4 bytes.
                verticesOffset
                ));

        float constants[] = ...;
        UINT constantsOffset = 0;
        ThrowIfFailed(
            SetDataToUploadHeap(
                constants, sizeof(float), sizeof(constants) / sizeof(float),
                D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                constantsOffset
                ));

        // Create vertex buffer views for the new binding model. 
        // Create constant buffer views for the new binding model. 
        // ...

        commandQueue->Execute(commandList);
        commandQueue->AdvanceFence();
    }

    HRESULT SuballocateFromHeap(SIZE_T uSize, UINT uAlign) {
        if (NOT_SUFFICIENT_MEMORY(uSize, uAlign)) {
            // Free up resources for frames processed by GPU; see Figure 2.
            auto fenceValue = _fence->GetCompleteFenceValue();
            FreeUpMemoryUntilFrame(fenceValue);

            while (NOT_SUFFICIENT_MEMORY(uSize, uAlign) && !frameOffsetQueue.empty()) {
                // Block until a new frame is processed by GPU, then free up more memory; see Figure 3.
                auto nextFenceValue = frameOffsetQueue.front().frameIndex;
                _fence->Sync(nextFenceValue);
                FreeUpMemoryUntilFrame(nextFenceValue);
            }
        }

        if (NOT_SUFFICIENT_MEMORY(uSize, uAlign)) {
            // Apps need to create a new Heap that is large enough for this resource.
            return E_HEAPNOTLARGEENOUGH;
        } else {
            // Update current data pointer for the new resource.
            auto ptr = frameOffsetQueue.back().pResourceOffset;
            m_pDataCur = reinterpret_cast<UINT8*>(
                Align(reinterpret_cast<SIZE_T>(m_pHDataCur), uAlign)
                );
            auto resourceOffset = Align(m_pDataCur, uAlign);
            auto currentFence = _fence->
            // Update frame offset queue if this is the first resource for a new frame; see Figure 4.
            UINT currentFrame = commandQueue->GetCurrentFence();
            if (frameOffsetQueue.empty()
                || frameOffsetQueue.back().frameIndex < currentFrame) {
                FrameResourceOffset offset = { currentFrame, resourceOffset };
                frameOffsetQueue.push(offset);
            }

            return S_OK;
        }
    }

    void FreeUpMemoryUntilFrame(UINT lastCompletedFenceValue) {
        while (!frameOffsetQueue.empty() && frameOffsetQueue.front().frameIndex <= lastCompletedFenceValue) {
            frameOffsetQueue.pop();
        }
    }
    void FreeUploadedSpace() {
        auto completedFence = _fence->GetCompleteFenceValue();
    }
private:
    std::queue<FrameResourceOffset> frameOffsetQueue;
    uint8 * m_pDataCur;
    Fence * _fence;

};

}