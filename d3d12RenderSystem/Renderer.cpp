#include "Renderer.h"

namespace d3d12RenderSystem {

auto Renderer::Prepare() -> void {

}

auto Renderer::RenderBegin() -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto & swapChainRenderTargets = _resourceManager->GetSwapChainRenderTargets();
    commandList->ResourceBarrier(1, swapChainRenderTargets.GetBarrierToRenderTarget());

    // Set necessary state.
    commandList->SetGraphicsRootSignature(_resourceManager->GetRootSignature());
    commandList->RSSetViewports(1, &_viewport);
    commandList->RSSetScissorRects(1, &_scissorRect);

    auto rtv = swapChainRenderTargets.GetCurrentRtv();
    commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

auto Renderer::RenderEnd() -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto & swapChainRenderTargets = _resourceManager->GetSwapChainRenderTargets();
    commandList->ResourceBarrier(1, swapChainRenderTargets.GetBarrierToPresent());

    ThrowIfFailed(commandList->Close());
    _resourceManager->GetFencedCommandQueue()->ExecuteCommandList(commandList, 1);
    swapChainRenderTargets.Present();
}

auto Renderer::RenderShape(core::Shape const * shape) -> void {
    // draw call
    auto mesh = shape->GetMesh();
    auto const& meshRenderData = _resourceManager->GetMeshData(mesh->_renderDataId);
    auto const& vertexIndexBuffer = _resourceManager->GetVertexIndexBuffer(meshRenderData.vertexIndexBufferIndex);

    // todo: only call the following 2 IASet* functions when necessary
    auto commandList = _resourceManager->GetCommandList();
    commandList->IASetVertexBuffers(0, 1, &vertexIndexBuffer.vbv);
    commandList->IASetIndexBuffer(&vertexIndexBuffer.ibv);
    commandList->DrawIndexedInstanced(meshRenderData.indexCount, 1, meshRenderData.indexOffset, meshRenderData.baseVertex, 0);
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