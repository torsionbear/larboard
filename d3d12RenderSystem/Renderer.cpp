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
    commandList->OMSetStencilRef(0);

    // render target
    auto rtv = swapChainRenderTargets.GetCurrentRtv();
    auto depthStencilBufferInfo = _resourceManager->GetDepthStencilBufferInfo(0);
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &depthStencilBufferInfo._cpuHandle);

    // null descriptors
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::DiffuseMap, _resourceManager->GetNullBufferInfo(0)._gpuHandle);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::NormalMap, _resourceManager->GetNullBufferInfo(1)._gpuHandle);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::SpecularMap, _resourceManager->GetNullBufferInfo(2)._gpuHandle);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::EmissiveMap, _resourceManager->GetNullBufferInfo(3)._gpuHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(depthStencilBufferInfo._cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

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

Renderer::Renderer(ResourceManager * resourceManager, unsigned int width, unsigned int height) {
    _resourceManager = resourceManager;
    _viewport.Width = static_cast<float>(width);
    _viewport.Height = static_cast<float>(height);
    _viewport.MaxDepth = 1.0f;
    _scissorRect.right = static_cast<LONG>(width);
    _scissorRect.bottom = static_cast<LONG>(height);
}

auto Renderer::RenderShape(core::Shape const * shape) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // transform
    auto const& transformBufferInfo = _resourceManager->GetTransformBufferInfo(shape->GetModel()->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Transform, transformBufferInfo._gpuHandle);
    // material
    auto const& materialBufferInfo = _resourceManager->GetMaterialBufferInfo(shape->GetMaterial()->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Material, materialBufferInfo._gpuHandle);
    // texture
    for (auto texture : shape->GetTextures()) {
        auto const& textureBufferInfo = _resourceManager->GetTextureBufferInfo(shape->GetTextures()[0]->_renderDataId);
        auto parameterIndex = RootSignatureParameterIndex::GetTextureRootSignatureParameterIndex(texture->GetType());
        commandList->SetGraphicsRootDescriptorTable(parameterIndex, textureBufferInfo._gpuHandle);
    }
    // vertex
    auto const& meshRenderData = _resourceManager->GetMeshDataInfo(shape->GetMesh()->_renderDataId);
    // todo: only call the following 2 IASet* functions when necessary
    commandList->IASetVertexBuffers(0, 1, &meshRenderData.vbv);
    commandList->IASetIndexBuffer(&meshRenderData.ibv);
    commandList->DrawIndexedInstanced(meshRenderData.indexCount, 1, meshRenderData.indexOffset, meshRenderData.baseVertex, 0);
}

auto Renderer::UseCamera(core::Camera const * camera) -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto const& cameraBufferInfo = _resourceManager->GetCameraBufferInfo(camera->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Camera, cameraBufferInfo._gpuHandle);
}

auto Renderer::UseLight() -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto const& lightBufferInfo = _resourceManager->GetLightBufferInfo();
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Light, lightBufferInfo._gpuHandle);
}

}