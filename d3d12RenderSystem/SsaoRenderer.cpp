#include "SsaoRenderer.h"

#include <array>
#include <random>

using std::array;

namespace d3d12RenderSystem {

auto SsaoRenderer::Prepare() -> void {
    Renderer::Prepare();
    _gBufferDiffuse = _resourceManager->CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, _viewport.Width, _viewport.Height, 1, &_gBufferDiffuseSrv);
    _gBufferNormal = _resourceManager->CreateRenderTarget(DXGI_FORMAT_R32G32B32A32_FLOAT, _viewport.Width, _viewport.Height, 1, &_gBufferNormalSrv);
    _gBufferSpecular = _resourceManager->CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, _viewport.Width, _viewport.Height, 1, &_gBufferSpecularSrv);
    _gBufferDepthStencil = _resourceManager->CreateDepthStencil(_viewport.Width, _viewport.Height, &_gBufferDepthStencilSrv);
    _ambientOcclusion = _resourceManager->CreateRenderTarget(DXGI_FORMAT_R32_FLOAT, _viewport.Width, _viewport.Height, 1, &_ambientOcclusionSrv);
    CreateSsaoDefaultPso();
    CreateSsaoPassPso();
    CreateLightingPassPso();
    GenerateRandomTexture(_randomTextureSize);
    PopulateSsaoData();
    LoadScreenQuad();
}

auto SsaoRenderer::Draw(core::Camera const * camera, core::SkyBox const * skyBox, core::Shape const * const * shapes, unsigned int shapeCount) -> void {
    DrawBegin();

    auto commandList = _resourceManager->GetCommandList();
    _currentPso = _ssaoDefaultPso.Get();
    commandList->SetPipelineState(_currentPso);
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    commandList->ClearRenderTargetView(_gBufferDiffuse._cpuHandle, clearColor, 0, nullptr);
    commandList->ClearRenderTargetView(_gBufferNormal._cpuHandle, clearColor, 0, nullptr);
    commandList->ClearRenderTargetView(_gBufferSpecular._cpuHandle, clearColor, 0, nullptr);
    commandList->ClearRenderTargetView(_ambientOcclusion._cpuHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(_gBufferDepthStencil._cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    UseCamera(camera);
    RenderSkyBox(skyBox);

    auto renderTargets = array< D3D12_CPU_DESCRIPTOR_HANDLE, 3>{_gBufferDiffuse._cpuHandle, _gBufferNormal._cpuHandle, _gBufferSpecular._cpuHandle};
    commandList->OMSetRenderTargets(renderTargets.size(), renderTargets.data(), FALSE, &_gBufferDepthStencil._cpuHandle);

    for (auto i = 0u; i < shapeCount; ++i) {
        RenderShape(shapes[i]);
    }

    // ssao pass
    if (_currentPso != _ssaoPassPso.Get()) {
        _currentPso = _ssaoPassPso.Get();
        commandList->SetPipelineState(_currentPso);
    }
    auto ssaoPassBarriers = std::array<CD3DX12_RESOURCE_BARRIER, 4> {
        CD3DX12_RESOURCE_BARRIER::Transition(_gBufferDiffuse._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(_gBufferNormal._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(_gBufferSpecular._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
            CD3DX12_RESOURCE_BARRIER::Transition(_gBufferDepthStencil._resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
    };
    commandList->ResourceBarrier(ssaoPassBarriers.size(), ssaoPassBarriers.data());

    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::DiffuseMap, _gBufferDiffuseSrv._gpuHandle); // diffuse
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::NormalMap, _gBufferNormalSrv._gpuHandle); // normal
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::SpecularMap, _gBufferSpecularSrv._gpuHandle); // specular
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::EmissiveMap, _gBufferDepthStencilSrv._gpuHandle); // depth texture
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Map4, _randomVectorTexture._gpuHandle); // random vector
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Material, _ssaoDataCbv._gpuHandle); // ssao data
    commandList->OMSetRenderTargets(1, &_ambientOcclusion._cpuHandle, FALSE, nullptr);
    //auto rtv = _resourceManager->GetSwapChainRenderTargets().GetCurrentRtv();
    //commandList->OMSetRenderTargets(1, &rtv, FALSE, &_depthStencil._cpuHandle);

    commandList->IASetVertexBuffers(0, 1, &_screenQuadVbv);
    commandList->IASetIndexBuffer(&_screenQuadIbv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

    // lighting pass
    if (_currentPso != _lightingPassPso.Get()) {
        _currentPso = _lightingPassPso.Get();
        commandList->SetPipelineState(_currentPso);
    }
    auto lightingPassBarriers = std::array<CD3DX12_RESOURCE_BARRIER, 1> {
        CD3DX12_RESOURCE_BARRIER::Transition(_ambientOcclusion._resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
    };
    commandList->ResourceBarrier(lightingPassBarriers.size(), lightingPassBarriers.data());
    commandList->SetGraphicsRootDescriptorTable(RootSignatureParameterIndex::Map4, _ambientOcclusionSrv._gpuHandle); // ambient occlusion
    UseLight();
    
    auto rtv = _resourceManager->GetSwapChainRenderTargets().GetCurrentRtv();
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &_depthStencil._cpuHandle);
    commandList->IASetVertexBuffers(0, 1, &_screenQuadVbv);
    commandList->IASetIndexBuffer(&_screenQuadIbv);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    
    DrawEnd();
}

auto SsaoRenderer::AllocateDescriptorHeap(
    unsigned int cameraCount,
    unsigned int meshCount,
    unsigned int modelCount,
    unsigned int textureCount,
    unsigned int materialCount,
    unsigned int skyBoxCount,
    unsigned int nullDescriptorCount) -> void {
    auto const lightDescriptorCount = 1u;
    auto const normalDsvCount = 1u;

    auto const gBufferRtvSrvCount = 3u;
    auto const gBufferDsvSrvCount = 1u;
    auto const ssaoRtvSrvCount = 1u;
    auto const ssaoCbvCount = 1u;
    auto const randomVectorTextureCount = 1u;

    _resourceManager->AllocDsvDescriptorHeap(normalDsvCount + gBufferDsvSrvCount);

    auto const cbvCount = cameraCount + modelCount + materialCount + lightDescriptorCount + ssaoCbvCount;
    auto const srvCount = textureCount + nullDescriptorCount + skyBoxCount + randomVectorTextureCount + gBufferRtvSrvCount + gBufferDsvSrvCount + ssaoRtvSrvCount;
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
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_default_v.hlsl", "vs_5_0");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_default_p.hlsl", "ps_5_0");

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

    _ssaoDefaultPso = _resourceManager->CreatePso(&psoDesc);
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
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_ssao_v.hlsl", "vs_5_0");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_ssao_p.hlsl", "ps_5_0");

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
    auto vs = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_lighting_v.hlsl", "vs_5_0");
    auto ps = _resourceManager->CompileShader("d3d12RenderSystem/shaders/ssao_lighting_p.hlsl", "ps_5_0");

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

auto SsaoRenderer::RenderShape(core::Shape const * shape) -> void {
    auto commandList = _resourceManager->GetCommandList();
    // pso
    if (_currentPso != _ssaoDefaultPso.Get()) {
        _currentPso = _ssaoDefaultPso.Get();
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

}