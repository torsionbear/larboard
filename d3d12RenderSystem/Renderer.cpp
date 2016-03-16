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
    _shadowMapDepthStencil = _resourceManager->CreateDepthStencil(_shadowMapSize(0), _shadowMapSize(1), &_shadowMapDepthStencilSrv);
    CreateDefaultPso();
    CreateTranslucentPso();
    CreateSkyBoxPso();
    CreateTerrainPso();
    CreateTerrainWireframePso();
    CreateShadowMapPso();
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

    // texture and texture index
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::SrvPsArray, _resourceManager->GetCbvSrvDescriptorHeap().GetGpuHandle(0));
    auto textureIndex = std::array<int, TextureIndex::count>{0, 0, 0, 0, 0, 0, 0};
    commandList->SetGraphicsRoot32BitConstants(RootSignatureParameterIndex::TextureIndex, textureIndex.size(), textureIndex.data(), 0);

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
    _wireframeMode = !_wireframeMode;
}

auto Renderer::ToggleBackFace() -> void {

}

auto Renderer::Draw(core::Viewpoint const* camera, core::SkyBox const* skyBox, core::Terrain const* terrain, core::Shape const*const* shapes, unsigned int shapeCount, core::Viewpoint const * shadowCastingLightViewpoint) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // render target
    auto rtv = _resourceManager->GetSwapChainRenderTargets().GetCurrentRtv();
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &_depthStencil._cpuHandle);
    commandList->RSSetViewports(1, &_viewport);
    commandList->RSSetScissorRects(1, &_scissorRect);

    auto const& shadowCastingLightDescriptorInfo = _resourceManager->GetCameraDescriptorInfo(shadowCastingLightViewpoint->GetRenderDataId());
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::CbvPs2, shadowCastingLightDescriptorInfo._gpuHandle);
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _shadowMapDepthStencilSrv._indexInDescriptorHeap, TextureIndex::slot4);

    UseViewpoint(camera);
    UseLight();
    if (skyBox != nullptr) {
        DrawSkyBox(skyBox);
    }
    if (terrain != nullptr) {
        DrawTerrain(terrain);
    }
    for (auto i = 0u; i < shapeCount; ++i) {
        DrawShapeWithPso(shapes[i], _defaultPso.Get());
    }
}

auto Renderer::DrawTranslucent(core::Shape const*const* shapes, unsigned int shapeCount) -> void {
    for (auto i = 0u; i < shapeCount; ++i) {
        DrawShapeWithPso(shapes[i], _translucentPso.Get());
    }
}

auto Renderer::AllocateDescriptorHeap(
    unsigned int cameraCount,
    unsigned int meshCount,
    unsigned int modelCount,
    unsigned int textureCount,
    unsigned int materialCount,
    unsigned int skyBoxCount,
    unsigned int terrainCount,
    unsigned int nullDescriptorCount) -> void {
    auto const ordinaryDsvCount = 1u;

    auto lightDescriptorCount = 1u;
    auto shadowCastingLightCount = 1u;
    _resourceManager->AllocDsvDescriptorHeap(ordinaryDsvCount + shadowCastingLightCount);

    auto cbvCount = cameraCount + modelCount + materialCount + lightDescriptorCount + terrainCount + shadowCastingLightCount;
    auto srvCount = textureCount + nullDescriptorCount + skyBoxCount + 2 * terrainCount + shadowCastingLightCount;
    _resourceManager->AllocCbvSrvDescriptorHeap(cbvCount + srvCount);
}

auto Renderer::DrawSkyBox(core::SkyBox const* skyBox) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // pso
    if (_currentPso != _skyBoxPso.Get()) {
        _currentPso = _skyBoxPso.Get();
        commandList->SetPipelineState(_currentPso);
    }
    // texture
    auto const& textureDescriptorInfo = _resourceManager->GetTextureDescriptorInfo(skyBox->GetRenderDataId());
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, textureDescriptorInfo._indexInDescriptorHeap, TextureIndex::slot0);
    // vertex
    auto const& skyBoxMeshInfo = _resourceManager->GetSkyBoxMeshInfo();
    // todo: only call the following 2 IASet* functions when necessary
    commandList->IASetVertexBuffers(0, 1, &skyBoxMeshInfo.vbv);
    commandList->IASetIndexBuffer(&skyBoxMeshInfo.ibv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(skyBoxMeshInfo.indexCount, 1, skyBoxMeshInfo.indexOffset, skyBoxMeshInfo.baseVertex, 0);
}

auto Renderer::DrawTerrain(core::Terrain const * terrain) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // pso
    auto targetPso = _wireframeMode ? _terrainWireframePso : _terrainPso;
    if (_currentPso != targetPso.Get()) {
        _currentPso = targetPso.Get();
        commandList->SetPipelineState(_currentPso);
    }
    // diffuse map & height map
    auto const& diffuseMapDescriptorInfo = _resourceManager->GetTextureDescriptorInfo(terrain->GetDiffuseMap()->_renderDataId);
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, diffuseMapDescriptorInfo._indexInDescriptorHeap, TextureIndex::slot0);
    auto const& heightMapDescriptorInfo = _resourceManager->GetTextureDescriptorInfo(terrain->GetHeightMap()->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::SrvAll1, heightMapDescriptorInfo._gpuHandle);
    // terrain cbv
    auto const& cbvDescriptorInfo = _resourceManager->GetTerrainCbv();
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::CbvAll, cbvDescriptorInfo._gpuHandle);
    // special tiles
    auto specialTiles = terrain->GetSpecialTiles();
    auto const& terrainMeshInfo = _resourceManager->GetTerrainMeshInfo();
    for (auto & mesh : specialTiles) {
        auto const& renderData = _resourceManager->GetMeshDataInfo(mesh->_renderDataId);
        auto vbvs = std::array<D3D12_VERTEX_BUFFER_VIEW, 2>{renderData.vbv, terrainMeshInfo.instanceVbv};
        commandList->IASetVertexBuffers(0, 2, vbvs.data());
        commandList->IASetIndexBuffer(&renderData.ibv);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        commandList->DrawIndexedInstanced(renderData.indexCount, 1, renderData.indexOffset, renderData.baseVertex, 0);
    }
    // normal tiles draw call
    auto vbvs = std::array<D3D12_VERTEX_BUFFER_VIEW, 2>{terrainMeshInfo.vbv, terrainMeshInfo.instanceVbv};
    commandList->IASetVertexBuffers(0, 2, vbvs.data());
    commandList->IASetIndexBuffer(&terrainMeshInfo.ibv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
    commandList->DrawIndexedInstanced(terrainMeshInfo.indexCount, terrain->GetTileCount() - specialTiles.size(), terrainMeshInfo.indexOffset, terrainMeshInfo.baseVertex, specialTiles.size());
}

auto Renderer::DrawShadowMap(core::Viewpoint const * viewpoint, core::Shape const*const* shapes, unsigned int shapeCount) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // render target
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_shadowMapDepthStencil._resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearDepthStencilView(_shadowMapDepthStencil._cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    commandList->OMSetRenderTargets(0, nullptr, FALSE, &_shadowMapDepthStencil._cpuHandle);
    // viewport & scissorRect
    auto viewport = D3D12_VIEWPORT{ 0.0f, 0.0f, static_cast<float>(_shadowMapSize(0)), static_cast<float>(_shadowMapSize(1)), 0.0f, 1.0f };
    auto scissorRect = D3D12_RECT{0, 0, _shadowMapSize(0), _shadowMapSize(1) };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    UseViewpoint(viewpoint);
    for (auto i = 0u; i < shapeCount; ++i) {
        DrawShapeWithPso(shapes[i], _shadowMapPso.Get());
    }
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_shadowMapDepthStencil._resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

}

auto Renderer::UseViewpoint(core::Viewpoint const * viewpoint) -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto const& cameraDescriptorInfo = _resourceManager->GetCameraDescriptorInfo(viewpoint->GetRenderDataId());
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Camera, cameraDescriptorInfo._gpuHandle);
}

auto Renderer::UseLight() -> void {
    auto commandList = _resourceManager->GetCommandList();
    auto const& lightDescriptorInfo = _resourceManager->GetLightDescriptorInfo();
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Light, lightDescriptorInfo._gpuHandle);
}

auto Renderer::DrawShapeWithPso(core::Shape const* shape, ID3D12PipelineState * pso) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // pso
    if (_currentPso != pso) {
        _currentPso = pso;
        commandList->SetPipelineState(_currentPso);
    }
    // transform
    auto const& transformDescriptorInfo = _resourceManager->GetTransformDescriptorInfo(shape->GetModel()->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Transform, transformDescriptorInfo._gpuHandle);
    // material
    auto const& materialDescriptorInfo = _resourceManager->GetMaterialDescriptorInfo(shape->GetMaterial()->_renderDataId);
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Material, materialDescriptorInfo._gpuHandle);
    // texture
    auto textureIndex = std::array<int, 4>{0, 0, 0, 0};
    for (auto i = 0u; i < shape->GetTextures().size(); ++i) {
        auto const& textureDescriptorInfo = _resourceManager->GetTextureDescriptorInfo(shape->GetTextures()[i]->_renderDataId);
        textureIndex[i] = textureDescriptorInfo._indexInDescriptorHeap;
    }
    commandList->SetGraphicsRoot32BitConstants(RootSignatureParameterIndex::TextureIndex, textureIndex.size(), textureIndex.data(), 0);
    // vertex
    auto const& meshRenderData = _resourceManager->GetMeshDataInfo(shape->GetMesh()->_renderDataId);
    // todo: only call the following 2 IASet* functions when necessary
    commandList->IASetVertexBuffers(0, 1, &meshRenderData.vbv);
    commandList->IASetIndexBuffer(&meshRenderData.ibv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(meshRenderData.indexCount, 1, meshRenderData.indexOffset, meshRenderData.baseVertex, 0);
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
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/default_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/default_p.hlsl", "ps_5_1");

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
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/skyBox_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/skyBox_p.hlsl", "ps_5_1");

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

auto Renderer::CreateTerrainPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 2> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "TILECOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/terrain_v.hlsl", "vs_5_1");
    auto hs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/terrain_h.hlsl", "hs_5_1");
    auto ds = _resourceManager->CompileShader("d3d12RenderSystem/shaders/terrain_d.hlsl", "ds_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/terrain_p.hlsl", "ps_5_1");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = _resourceManager->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    psoDesc.HS = CD3DX12_SHADER_BYTECODE(hs.Get());
    psoDesc.DS = CD3DX12_SHADER_BYTECODE(ds.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    psoDesc.NumRenderTargets = 3;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    _terrainPso = _resourceManager->CreatePso(&psoDesc);
}

auto Renderer::CreateTerrainWireframePso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 2> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "TILECOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
    // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/terrain_v.hlsl", "vs_5_1");
    auto hs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/terrain_h.hlsl", "hs_5_1");
    auto ds = _resourceManager->CompileShader("d3d12RenderSystem/shaders/terrain_d.hlsl", "ds_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/terrain_p.hlsl", "ps_5_1");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = _resourceManager->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    psoDesc.HS = CD3DX12_SHADER_BYTECODE(hs.Get());
    psoDesc.DS = CD3DX12_SHADER_BYTECODE(ds.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    psoDesc.NumRenderTargets = 3;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    _terrainWireframePso = _resourceManager->CreatePso(&psoDesc);
}

auto Renderer::CreateTranslucentPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 3> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    // rasterizer
    auto rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    // depth stencil
    auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // turn off depth write
    // blend
    auto blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    blendDesc.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC{
        TRUE,FALSE,
        D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/default_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/default_p.hlsl", "ps_5_1");

    auto psoDesc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = _resourceManager->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    _translucentPso = _resourceManager->CreatePso(&psoDesc);
}

auto Renderer::CreateShadowMapPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 3> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    // rasterizer
    auto rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/default_v.hlsl", "vs_5_1");

    auto psoDesc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = _resourceManager->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(0, 0);
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    psoDesc.SampleDesc.Count = 1;

    _shadowMapPso = _resourceManager->CreatePso(&psoDesc);
}

}