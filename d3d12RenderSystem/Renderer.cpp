#include "Renderer.h"

#include <array>

using std::array;

namespace d3d12RenderSystem {

auto Renderer::Prepare() -> void {
}

auto Renderer::DrawBegin() -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto & swapChainRenderTargets = _resourceManager->GetSwapChainRenderTargets();
    commandList->ResourceBarrier(1, swapChainRenderTargets.GetBarrierToRenderTarget());

    // Set necessary state.
    // root signature
    commandList->SetGraphicsRootSignature(_resourceManager->GetRootSignature());
    // descriptor heaps. Only shader visible descriptor heaps need to be bound to command lists.
    auto heaps = array<ID3D12DescriptorHeap *, 1>{
            _resourceManager->GetCbvSrvDescriptorHeap().GetHeap()
    };
    commandList->SetDescriptorHeaps(heaps.size(), heaps.data());
    // viewport & scissorRect
    commandList->RSSetViewports(1, &_viewport);
    commandList->RSSetScissorRects(1, &_scissorRect);

    // render target
    auto rtv = swapChainRenderTargets.GetCurrentRtv();
    auto depthStencilBufferHandle = _resourceManager->GetDepthStencilBufferHandle(0);
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &depthStencilBufferHandle._cpuHandle);


    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

auto Renderer::DrawEnd() -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto & swapChainRenderTargets = _resourceManager->GetSwapChainRenderTargets();
    commandList->ResourceBarrier(1, swapChainRenderTargets.GetBarrierToPresent());

    ThrowIfFailed(commandList->Close());
    _resourceManager->GetFencedCommandQueue()->ExecuteCommandList(commandList, 1);
    swapChainRenderTargets.Present();
}

auto Renderer::ToggleWireframe() -> void {

}

auto Renderer::ToggleBackFace() -> void {

}

auto Renderer::RenderShape(core::Shape const * shape) -> void {
    // draw call
    auto const& meshRenderData = _resourceManager->GetMeshData(shape->GetMesh()->_renderDataId);
    auto const& vertexIndexBuffer = _resourceManager->GetVertexIndexBuffer(meshRenderData.vertexIndexBufferIndex);
    auto const& transformBufferHandle = _resourceManager->GetTransformBufferHandle(shape->GetModel()->_renderDataId);

    // todo: only call the following 2 IASet* functions when necessary
    auto commandList = _resourceManager->GetCommandList();
    commandList->IASetVertexBuffers(0, 1, &vertexIndexBuffer.vbv);
    commandList->IASetIndexBuffer(&vertexIndexBuffer.ibv);
    commandList->SetGraphicsRootDescriptorTable(1, transformBufferHandle._gpuHandle);
    commandList->DrawIndexedInstanced(meshRenderData.indexCount, 1, meshRenderData.indexOffset, meshRenderData.baseVertex, 0);
}

auto Renderer::UseCamera(core::Camera const * camera) -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto const& cameraBufferHandle = _resourceManager->GetCameraBufferHandle(camera->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(0, cameraBufferHandle._gpuHandle);
}

auto Renderer::Init(ResourceManager * resourceManager, unsigned int width, unsigned int height) -> void {
    _resourceManager = resourceManager;
    _viewport.Width = static_cast<float>(width);
    _viewport.Height = static_cast<float>(height);
    _viewport.MaxDepth = 1.0f;
    _scissorRect.right = static_cast<LONG>(width);
    _scissorRect.bottom = static_cast<LONG>(height);
}

}