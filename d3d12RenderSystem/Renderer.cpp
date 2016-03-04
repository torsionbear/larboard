#include "Renderer.h"

#include <array>

using std::array;

namespace d3d12RenderSystem {

Renderer::Renderer(ResourceManager * resourceManager, unsigned int width, unsigned int height) {
    _resourceManager = resourceManager;
    _viewport.Width = static_cast<float>(width);
    _viewport.Height = static_cast<float>(height);
    _viewport.MaxDepth = 1.0f;
    _scissorRect.right = static_cast<LONG>(width);
    _scissorRect.bottom = static_cast<LONG>(height);
}

auto Renderer::Prepare() -> void {
    _depthStencil = _resourceManager->CreateDepthStencil(_viewport.Width, _viewport.Height, nullptr);
    CreateDefaultPso();
    CreateSkyBoxPso();
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
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &_depthStencil._cpuHandle);

    // null descriptors
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::DiffuseMap, _resourceManager->GetNullDescriptorInfo(0)._gpuHandle);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::NormalMap, _resourceManager->GetNullDescriptorInfo(1)._gpuHandle);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::SpecularMap, _resourceManager->GetNullDescriptorInfo(2)._gpuHandle);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::EmissiveMap, _resourceManager->GetNullDescriptorInfo(3)._gpuHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(_depthStencil._cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    _currentPso = _defaultPso.Get();
    commandList->SetPipelineState(_currentPso);
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

auto Renderer::Draw(core::Camera const* camera, core::SkyBox const* skyBox, core::Shape const*const* shapes, unsigned int shapeCount) -> void {
    DrawBegin();
    UseCamera(camera);
    UseLight();
    RenderSkyBox(skyBox);
    for (auto i = 0u; i < shapeCount; ++i) {
        RenderShape(shapes[i]);
    }
    DrawEnd();
}

auto Renderer::AllocateDescriptorHeap(
    unsigned int cameraCount,
    unsigned int meshCount,
    unsigned int modelCount,
    unsigned int textureCount,
    unsigned int materialCount,
    unsigned int skyBoxCount,
    unsigned int nullDescriptorCount) -> void {
    _resourceManager->AllocDsvDescriptorHeap(1);
    auto const lightDescriptorCount = 1u;
    auto const cbvCount = cameraCount + modelCount + materialCount + lightDescriptorCount;
    auto const srvCount = textureCount + nullDescriptorCount + skyBoxCount;
    _resourceManager->AllocCbvSrvDescriptorHeap(cbvCount + srvCount);
}

auto Renderer::RenderShape(core::Shape const * shape) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // pso
    if (_currentPso != _defaultPso.Get()) {
        _currentPso = _defaultPso.Get();
        commandList->SetPipelineState(_currentPso);
    }
    // transform
    auto const& transformDescriptorInfo = _resourceManager->GetTransformDescriptorInfo(shape->GetModel()->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Transform, transformDescriptorInfo._gpuHandle);
    // material
    auto const& materialDescriptorInfo = _resourceManager->GetMaterialDescriptorInfo(shape->GetMaterial()->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Material, materialDescriptorInfo._gpuHandle);
    // texture
    for (auto texture : shape->GetTextures()) {
        auto const& textureDescriptorInfo = _resourceManager->GetTextureDescriptorInfo(shape->GetTextures()[0]->_renderDataId);
        auto parameterIndex = RootSignatureParameterIndex::GetTextureRootSignatureParameterIndex(texture->GetType());
        commandList->SetGraphicsRootDescriptorTable(parameterIndex, textureDescriptorInfo._gpuHandle);
    }
    // vertex
    auto const& meshRenderData = _resourceManager->GetMeshDataInfo(shape->GetMesh()->_renderDataId);
    // todo: only call the following 2 IASet* functions when necessary
    commandList->IASetVertexBuffers(0, 1, &meshRenderData.vbv);
    commandList->IASetIndexBuffer(&meshRenderData.ibv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(meshRenderData.indexCount, 1, meshRenderData.indexOffset, meshRenderData.baseVertex, 0);
}

auto Renderer::RenderSkyBox(core::SkyBox const* skyBox) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // pso
    if (_currentPso != _skyBoxPso.Get()) {
        _currentPso = _skyBoxPso.Get();
        commandList->SetPipelineState(_currentPso);
    }
    // texture
    auto textureDescriptorInfo = _resourceManager->GetTextureDescriptorInfo(skyBox->GetRenderDataId());
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::DiffuseMap, textureDescriptorInfo._gpuHandle);
    // vertex
    auto const& skyBoxMeshInfo = _resourceManager->GetSkyBoxMeshInfo();
    // todo: only call the following 2 IASet* functions when necessary
    commandList->IASetVertexBuffers(0, 1, &skyBoxMeshInfo.vbv);
    commandList->IASetIndexBuffer(&skyBoxMeshInfo.ibv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(skyBoxMeshInfo.indexCount, 1, skyBoxMeshInfo.indexOffset, skyBoxMeshInfo.baseVertex, 0);
}


auto Renderer::UseCamera(core::Camera const * camera) -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto const& cameraDescriptorInfo = _resourceManager->GetCameraDescriptorInfo(camera->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Camera, cameraDescriptorInfo._gpuHandle);
}

auto Renderer::UseLight() -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto const& lightDescriptorInfo = _resourceManager->GetLightDescriptorInfo();
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Light, lightDescriptorInfo._gpuHandle);
}

auto Renderer::CreateDefaultPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 3> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/default_v.hlsl", "vs_5_0");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/default_p.hlsl", "ps_5_0");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = _resourceManager->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    _defaultPso = _resourceManager->CreatePso(&psoDesc);
}

auto Renderer::CreateSkyBoxPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 1> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;  // turn off depth write
    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/skyBox_v.hlsl", "vs_5_0");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/skyBox_p.hlsl", "ps_5_0");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = _resourceManager->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    _skyBoxPso = _resourceManager->CreatePso(&psoDesc);
}

}