#include "SsaoRenderer.h"

#include <array>
#include <random>

using std::array;

namespace d3d12RenderSystem {

auto SsaoRenderer::Prepare() -> void {
    Renderer::Prepare();
    auto width = static_cast<unsigned int>(_viewport.Width);
    auto height = static_cast<unsigned int>(_viewport.Height);
    _gBufferDiffuse = _resourceManager->CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, &_gBufferDiffuseSrv);
    _gBufferNormal = _resourceManager->CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, width, height, 1, &_gBufferNormalSrv);
    _gBufferSpecular = _resourceManager->CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, &_gBufferSpecularSrv);
    _gBufferDepthStencil = _resourceManager->CreateDepthStencil(width, height, &_gBufferDepthStencilSrv);
    _ambientOcclusion = _resourceManager->CreateRenderTarget(DXGI_FORMAT_R32_FLOAT, width, height, 1, &_ambientOcclusionSrv);
    CreateSsaoDefaultPso();
    CreateSsaoPassPso();
    CreateLightingPassPso();
    CreateAmbientLightPso();
    CreateDirectionalLightPso();
    CreatePointLightPso();
    CreateSpotLightPso();
    GenerateRandomTexture(_randomTextureSize);
    PopulateSsaoData();
    LoadScreenQuad();
    LoadPointLightVolume();
}

auto SsaoRenderer::Draw(
    core::Viewpoint const* camera,
    core::SkyBox const* skyBox,
    core::Terrain const* terrain,
    core::Shape const*const* shapes,
    unsigned int shapeCount,
    core::Viewpoint const * shadowCastingLightViewpoint,
    core::AmbientLight * ambientLight,
    core::DirectionalLight ** directionalLights, unsigned int directionalLightCount,
    core::PointLight ** pointLights, unsigned int pointLightCount,
    core::SpotLight ** spotLights, unsigned int spotLightCount) -> void {
    auto commandList = _resourceManager->GetCommandList();

    auto renderTargets = array< D3D12_CPU_DESCRIPTOR_HANDLE, 3>{_gBufferDiffuse._cpuHandle, _gBufferNormal._cpuHandle, _gBufferSpecular._cpuHandle};
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    commandList->ClearRenderTargetView(_gBufferDiffuse._cpuHandle, clearColor, 0, nullptr);
    commandList->ClearRenderTargetView(_gBufferNormal._cpuHandle, clearColor, 0, nullptr);
    commandList->ClearRenderTargetView(_gBufferSpecular._cpuHandle, clearColor, 0, nullptr);
    commandList->ClearRenderTargetView(_ambientOcclusion._cpuHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(_gBufferDepthStencil._cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    commandList->OMSetRenderTargets(renderTargets.size(), renderTargets.data(), FALSE, &_gBufferDepthStencil._cpuHandle);
    commandList->RSSetViewports(1, &_viewport);
    commandList->RSSetScissorRects(1, &_scissorRect);

    UseViewpoint(commandList, camera);

    DrawShapes(commandList);
    if (terrain != nullptr) {
        DrawTerrain(commandList, terrain);
    }

    // ssao pass
    commandList->SetPipelineState(_ssaoPassPso.Get());

    auto ssaoPassBarriers = std::array<CD3DX12_RESOURCE_BARRIER, 4> {
        CD3DX12_RESOURCE_BARRIER::Transition(_gBufferDiffuse._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(_gBufferNormal._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(_gBufferSpecular._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(_gBufferDepthStencil._resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
    };
    commandList->ResourceBarrier(ssaoPassBarriers.size(), ssaoPassBarriers.data());

    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _gBufferDiffuseSrv._indexInDescriptorHeap, TextureIndex::slot0); // diffuse
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _gBufferNormalSrv._indexInDescriptorHeap, TextureIndex::slot1); // normal
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _gBufferSpecularSrv._indexInDescriptorHeap, TextureIndex::slot2); // specular
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _gBufferDepthStencilSrv._indexInDescriptorHeap, TextureIndex::slot6); // depth
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _randomVectorTexture._indexInDescriptorHeap, TextureIndex::slot5); // random vector
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Material, _ssaoDataCbv._gpuHandle); // ssao data

    commandList->OMSetRenderTargets(1, &_ambientOcclusion._cpuHandle, FALSE, nullptr);

    commandList->IASetVertexBuffers(0, 1, &_screenQuadVbv);
    commandList->IASetIndexBuffer(&_screenQuadIbv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    
    /*
    // lighting pass    
    commandList->SetPipelineState(_lightingPassPso.Get());

    // shadow casting light's cbv and shadow map's srv
    auto const& shadowCastingLightDescriptorInfo = _resourceManager->GetCameraDescriptorInfo(shadowCastingLightViewpoint->GetRenderDataId());
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::CbvPs2, shadowCastingLightDescriptorInfo._gpuHandle);
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _shadowMapDepthStencilSrv._indexInDescriptorHeap, TextureIndex::slot4); // shadow map

    auto lightingPassBarriers = std::array<CD3DX12_RESOURCE_BARRIER, 1> {
        CD3DX12_RESOURCE_BARRIER::Transition(_ambientOcclusion._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
    };
    commandList->ResourceBarrier(lightingPassBarriers.size(), lightingPassBarriers.data());
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _ambientOcclusionSrv._indexInDescriptorHeap, TextureIndex::slot5); // occlusion
    UseLight();
    
    commandList->OMSetRenderTargets(1, &swapChainRtv, FALSE, &_depthStencil._cpuHandle);
    commandList->IASetVertexBuffers(0, 1, &_screenQuadVbv);
    commandList->IASetIndexBuffer(&_screenQuadIbv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    */

    // deferred pass
    auto swapChainRtv = _resourceManager->GetSwapChainRenderTargets().GetCurrentRtv();
    commandList->OMSetRenderTargets(1, &swapChainRtv, FALSE, &_gBufferDepthStencil._cpuHandle);
    // ambient
    auto lightingPassBarriers = std::array<CD3DX12_RESOURCE_BARRIER, 1> {
        CD3DX12_RESOURCE_BARRIER::Transition(_ambientOcclusion._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
    };
    commandList->ResourceBarrier(lightingPassBarriers.size(), lightingPassBarriers.data());
    DrawAmbientLight(commandList, ambientLight, _ambientOcclusionSrv);
    DrawDirectionalLights(commandList, directionalLights, directionalLightCount);
    DrawPointLights(commandList, pointLights, pointLightCount);
    DrawSpotLights(commandList, spotLights, spotLightCount);

    if (skyBox != nullptr) {
        DrawSkyBox(commandList);
    }
}

auto SsaoRenderer::AllocateDescriptorHeap(
    unsigned int cameraCount,
    unsigned int meshCount,
    unsigned int movableCount,
    unsigned int textureCount,
    unsigned int materialCount,
    unsigned int skyBoxCount,
    unsigned int terrainCount,
    unsigned int directionalLightCount,
    unsigned int pointLightCount,
    unsigned int spotLightCount,
    unsigned int nullDescriptorCount) -> void {
    auto const lightDescriptorCount = 1u;
    auto ambientLightCount = 1u;
    auto shadowCastingLightCount = 1u;
    auto const ordinaryDsvCount = 1u;

    auto const gBufferRtvSrvCount = 3u;
    auto const gBufferDsvSrvCount = 1u;
    auto const ssaoRtvSrvCount = 1u;
    auto const ssaoCbvCount = 1u;
    auto const randomVectorTextureCount = 1u;

    _resourceManager->AllocDsvDescriptorHeap(ordinaryDsvCount + gBufferDsvSrvCount + shadowCastingLightCount);

    auto const cbvCount = cameraCount + movableCount + materialCount + lightDescriptorCount + ambientLightCount + directionalLightCount + pointLightCount + spotLightCount + ssaoCbvCount + terrainCount + shadowCastingLightCount;
    auto const srvCount = textureCount + nullDescriptorCount + skyBoxCount + randomVectorTextureCount + gBufferRtvSrvCount + gBufferDsvSrvCount + ssaoRtvSrvCount + 2 * terrainCount + shadowCastingLightCount;
    _resourceManager->AllocCbvSrvDescriptorHeap(cbvCount + srvCount);
    _resourceManager->AllocRtvDescriptorHeap(gBufferRtvSrvCount + ssaoRtvSrvCount);
}

auto SsaoRenderer::GenerateRandomTexture(unsigned int textureSize) -> void {
    std::random_device randomDevice;
    auto randomFloats = std::uniform_real_distribution<Float32>(-1.0, 1.0);
    auto randomEngine = std::default_random_engine(randomDevice());
    auto randomVectors = vector<core::Vector4f>{};
    for (auto i = 0u; i < textureSize * textureSize; ++i) {
        auto randomVector = core::Vector4f{
            randomFloats(randomEngine),
            randomFloats(randomEngine),
            0.0f,
            0.0f,
        };
        randomVectors.push_back(randomVector);
    }

    _randomVectorTexture = _resourceManager->CreateTexture2d(DXGI_FORMAT_R32G32B32A32_FLOAT, textureSize, textureSize, randomVectors.data(), randomVectors.size(), sizeof(core::Vector4f));
}

auto SsaoRenderer::GenerateSamples(unsigned int sampleCount, core::Vector2f sampleDistanceRange) -> void {
    std::random_device randomDevice;
    auto randomFloats = std::uniform_real_distribution<Float32>(0.0, 1.0);
    auto randomEngine = std::default_random_engine{ randomDevice() };
    
    auto ssaoKernel = vector<core::Vector3f>{};
    for (auto i = 0u; i < sampleCount; ++i) {
        auto sample = core::Vector3f{
            randomFloats(randomEngine) * 2.0f - 1.0f,
            randomFloats(randomEngine) * 2.0f - 1.0f,
            randomFloats(randomEngine)
        };
        sample = core::Normalize(sample);
        sample = sample * randomFloats(randomEngine);

        auto scale = static_cast<Float32>(i) / sampleCount;
        scale = sampleDistanceRange(0) + (sampleDistanceRange(1) - sampleDistanceRange(0)) * scale * scale;
        sample = sample * scale;
        _ssaoData.samples[i] = core::Vector4f{ sample(0), sample(1), sample(2), 1.0f };
    }
}

auto SsaoRenderer::PopulateSsaoData() -> void {
    //_ssaoData.randomVectorTextureSize = core::Vector2i{ static_cast<int>(_randomTextureSize), static_cast<int>(_randomTextureSize) };
    _ssaoData.occlusionTextureSize = core::Vector2i{ static_cast<int>(_viewport.Width), static_cast<int>(_viewport.Height) };
    _ssaoData.sampleCount = _sampleCount;
    GenerateSamples(_sampleCount, core::Vector2f{ 0.1f, 1.0f });
    _ssaoDataCbv = _resourceManager->UploadConstantBufferData(sizeof(SsaoData), &_ssaoData);
}

auto SsaoRenderer::CreateSsaoDefaultPso() -> void {
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
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_default_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_default_p.hlsl", "ps_5_1");

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
    psoDesc.NumRenderTargets = 3;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    _defaultPso = _resourceManager->CreatePso(&psoDesc);
}

auto SsaoRenderer::CreateSsaoPassPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 1> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;

    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_ssao_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_ssao_p.hlsl", "ps_5_1");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = _resourceManager->GetRootSignature();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
    psoDesc.SampleDesc.Count = 1;

    _ssaoPassPso = _resourceManager->CreatePso(&psoDesc);
}

auto SsaoRenderer::CreateLightingPassPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 1> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);

    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_lighting_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_lighting_p.hlsl", "ps_5_1");

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

    _lightingPassPso = _resourceManager->CreatePso(&psoDesc);
}

auto SsaoRenderer::CreateAmbientLightPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 1> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // turn off depth write
                                                                   
    // blend
    auto blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    blendDesc.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC{
        TRUE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_ambientLight_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_ambientLight_p.hlsl", "ps_5_1");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
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

    _ambientLightPso = _resourceManager->CreatePso(&psoDesc);
}

auto SsaoRenderer::CreateDirectionalLightPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 1> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
    rasterizerDesc.FrontCounterClockwise = TRUE;
    // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // turn off depth write

    // blend
    auto blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    blendDesc.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC{
        TRUE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_directionalLight_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_directionalLight_p.hlsl", "ps_5_1");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
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

    _directionalLightPso = _resourceManager->CreatePso(&psoDesc);
}

auto SsaoRenderer::CreatePointLightPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 1> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT); // draw back faces
    // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // turn off depth write
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL; // do not render "floating in the air" portion of light volume
    // blend
    auto blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    blendDesc.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC{
        TRUE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_pointLight_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_pointLight_p.hlsl", "ps_5_1");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
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

    _pointLightPso = _resourceManager->CreatePso(&psoDesc);
}

auto SsaoRenderer::CreateSpotLightPso() -> void {
    // vertex attribute
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 1> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
    // rasterizer
    CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT); // draw back faces
                                                           // depth stencil
    CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // turn off depth write
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL; // do not render "floating in the air" portion of light volume
                                                                      // blend
    auto blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    blendDesc.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC{
        TRUE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    // shader
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_spotLight_v.hlsl", "vs_5_1");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_spotLight_p.hlsl", "ps_5_1");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
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

    _spotLightPso = _resourceManager->CreatePso(&psoDesc);
}

auto SsaoRenderer::LoadScreenQuad() -> void {
    auto vertexData = array<core::Vector2f, 4>{ 
        core::Vector2f{ -1, -1 }, 
            core::Vector2f{ 1, -1 }, 
            core::Vector2f{ 1, 1 },
            core::Vector2f{ -1, 1 } };
    auto indexData = vector<unsigned int>{ 0, 1, 2, 0, 2, 3 };

    _screenQuadVbv = _resourceManager->UploadVertexData(vertexData.size() * sizeof(core::Vector2f), sizeof(core::Vector2f), vertexData.data());
    _screenQuadIbv = _resourceManager->UploadIndexData(indexData.size() * sizeof(unsigned int), indexData.data());
}

auto SsaoRenderer::LoadPointLightVolume() -> void {
    auto pointLightVolume = core::PointLight::GetLightVolume();
    _resourceManager->LoadMeshes<core::VertexC3>(&pointLightVolume, 1);
}

auto SsaoRenderer::DrawAmbientLight(ID3D12GraphicsCommandList * commandList, core::AmbientLight * ambientLight, DescriptorInfo ambientOcclusionSrv) -> void {
    commandList->SetPipelineState(_ambientLightPso.Get());
    // occlusion
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, ambientOcclusionSrv._indexInDescriptorHeap, TextureIndex::slot5);
    // ssao data
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Material, _ssaoDataCbv._gpuHandle);
    // ambient light
    auto ambientLightDescriptorInfo = _resourceManager->GetAmbientLightDescriptorInfo(ambientLight->GetRenderDataId());
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Light, ambientLightDescriptorInfo._gpuHandle); // ssao data

    commandList->IASetVertexBuffers(0, 1, &_screenQuadVbv);
    commandList->IASetIndexBuffer(&_screenQuadIbv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

auto SsaoRenderer::DrawDirectionalLights(ID3D12GraphicsCommandList * commandList, core::DirectionalLight ** directionalLights, unsigned int directionalLightCount) -> void {
    commandList->SetPipelineState(_directionalLightPso.Get());

    commandList->IASetVertexBuffers(0, 1, &_screenQuadVbv);
    commandList->IASetIndexBuffer(&_screenQuadIbv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // shadow casting light's cbv and shadow map's srv
    auto const& shadowCastingLightDescriptorInfo = _resourceManager->GetCameraDescriptorInfo(directionalLights[0]->core::Viewpoint::GetRenderDataId());
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::CbvPs2, shadowCastingLightDescriptorInfo._gpuHandle);
    commandList->SetGraphicsRoot32BitConstant(RootSignatureParameterIndex::TextureIndex, _shadowMapDepthStencilSrv._indexInDescriptorHeap, TextureIndex::slot4); // shadow map

    for (auto i = 0u; i < directionalLightCount; ++i) {
        auto directionalLight = directionalLights[i];
        // transform
        auto const& transformDescriptorInfo = _resourceManager->GetTransformDescriptorInfo(directionalLight->core::Movable::GetRenderDataId());
        commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Transform, transformDescriptorInfo._gpuHandle);
        // directionalLight
        auto const& lightDescriptorInfo = _resourceManager->GetDirectionalLightDescriptorInfo(directionalLight->GetRenderDataId());
        commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Light, lightDescriptorInfo._gpuHandle);
        // vertex
        commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
}

auto SsaoRenderer::DrawPointLights(ID3D12GraphicsCommandList * commandList, core::PointLight ** pointLights, unsigned int pointLightCount) -> void {
    commandList->SetPipelineState(_pointLightPso.Get());
    for (auto i = 0u; i < pointLightCount; ++i) {
        auto pointLight = pointLights[i];
        // transform
        auto const& transformDescriptorInfo = _resourceManager->GetTransformDescriptorInfo(pointLight->core::Movable::GetRenderDataId());
        commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Transform, transformDescriptorInfo._gpuHandle);
        // pointLight
        auto const& lightDescriptorInfo = _resourceManager->GetPointLightDescriptorInfo(pointLight->GetRenderDataId());
        commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Light, lightDescriptorInfo._gpuHandle);
        // vertex
        auto const& meshRenderData = _resourceManager->GetMeshDataInfo(pointLight->GetLightVolume()->GetRenderDataId());

        commandList->IASetVertexBuffers(0, 1, &meshRenderData.vbv);
        commandList->IASetIndexBuffer(&meshRenderData.ibv);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->DrawIndexedInstanced(meshRenderData.indexCount, 1, meshRenderData.indexOffset, meshRenderData.baseVertex, 0);
    }
}

auto SsaoRenderer::DrawSpotLights(ID3D12GraphicsCommandList * commandList, core::SpotLight ** spotLights, unsigned int spotLightCount) -> void {
    commandList->SetPipelineState(_spotLightPso.Get());
    for (auto i = 0u; i < spotLightCount; ++i) {
        auto spotLight = spotLights[i];
        // transform
        auto const& transformDescriptorInfo = _resourceManager->GetTransformDescriptorInfo(spotLight->core::Movable::GetRenderDataId());
        commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Transform, transformDescriptorInfo._gpuHandle);
        // spotLight
        auto const& lightDescriptorInfo = _resourceManager->GetSpotLightDescriptorInfo(spotLight->GetRenderDataId());
        commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Light, lightDescriptorInfo._gpuHandle);
        // vertex
        auto const& meshRenderData = _resourceManager->GetMeshDataInfo(spotLight->GetLightVolume()->GetRenderDataId());
        // todo: only call the following 2 IASet* functions when necessary
        commandList->IASetVertexBuffers(0, 1, &meshRenderData.vbv);
        commandList->IASetIndexBuffer(&meshRenderData.ibv);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->DrawIndexedInstanced(meshRenderData.indexCount, 1, meshRenderData.indexOffset, meshRenderData.baseVertex, 0);
    }
}

}