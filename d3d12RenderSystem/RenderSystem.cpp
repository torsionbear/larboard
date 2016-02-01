#include "RenderSystem.h"

#include <assert.h>
#include <array>

#include <D3Dcompiler.h>

#include "d3dx12.h"
#include "Common.h"

using Microsoft::WRL::ComPtr;
using std::array;

namespace d3d12RenderSystem {

auto RenderSystem::Render(core::Shape const* shape) -> void {
    // shaderProgram
    //shape->GetShaderProgram()->Use();
    //if (_currentShaderProgram != shape.GetShaderProgram()) {
    //    shape.GetShaderProgram()->Use();
    //    _currentShaderProgram = shape.GetShaderProgram();
    //}

    // transform
    //auto model = shape->GetModel();
    //glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Transform), model->GetUbo(), model->GetUboOffset(), Model::ShaderData::Size());

    // material
    //auto material = shape->GetMaterial();
    //glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Material), material->GetUbo(), material->GetUboOffset(), Material::ShaderData::Size());

    // texture
    //for (auto & texture : shape->GetTextures()) {
    //    UseTexture(texture, texture->GetType());
    //}

    // draw call
    auto mesh = shape->GetMesh();
    auto const& meshRenderData = _resourceManager.GetMeshData(mesh->_renderDataId);

    // todo: only call the following 2 IASet* functions when necessary
    _commandList->IASetVertexBuffers(0, 1, &meshRenderData.vbv);
    _commandList->IASetIndexBuffer(&meshRenderData.ibv);
    _commandList->DrawIndexedInstanced(meshRenderData.indexCount, 1, meshRenderData.indexOffset, meshRenderData.baseVertex, 0);
}

auto RenderSystem::Update() -> void {
}

auto RenderSystem::RenderBegin() -> void {
    _frameManager.FrameBegin();

    // reset command list immediately after submission to reuse the allocated memory.
    ThrowIfFailed(_commandList->Reset(_frameManager.GetCurrentFrameResource()->GetCommandAllocator(), _defaultPso.Get()));

    auto barrier = _swapChainRenderTargets.GetBarrierToRenderTarget();
    _commandList->ResourceBarrier(1, barrier);

    // Set necessary state.
    _commandList->SetGraphicsRootSignature(_rootSignature.Get());
    _commandList->RSSetViewports(1, &_viewport);
    _commandList->RSSetScissorRects(1, &_scissorRect);

    auto rtv = _swapChainRenderTargets.GetCurrentRtv();
    _commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    _commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

auto RenderSystem::RenderEnd() -> void {
    _commandList->ResourceBarrier(1, _swapChainRenderTargets.GetBarrierToPresent());

    // submit command list
    auto list = static_cast<ID3D12CommandList *>(_commandList.Get());
    ThrowIfFailed(_commandList->Close());
    _commandQueue->ExecuteCommandLists(1, &list);

    _swapChainRenderTargets.Present();
    _frameManager.FrameEnd();
}

auto RenderSystem::Init() -> void {
    EnableDebugLayer();
    auto factory = CreateFactory();
    _device = CreateDevice(factory.Get());
    _commandQueue = CreateCommandQueue(_device.Get());
    _swapChainRenderTargets.Init(factory.Get(), _device.Get(), _commandQueue.Get(), _renderWindow->GetWidth(), _renderWindow->GetHeight(), _renderWindow->GetHwnd());
    _resourceManager.Init(_device.Get());
    _srvHeap = CreateSrvHeap(_device.Get(), 1);

    _fence.Init(_device.Get(), _commandQueue.Get());

    auto frameResources = std::vector<FrameResource>(2);
    for (auto & frameResource : frameResources) {
        frameResource.Init(_device.Get());
    }
    _frameManager.Init(&_fence, move(frameResources));

}

auto RenderSystem::EnableDebugLayer() -> void {
#if defined(_DEBUG)
    // 0. enable d3d12 debug layer
    auto d3d12Debug = ComPtr<ID3D12Debug>{ nullptr };
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug)))) {
        d3d12Debug->EnableDebugLayer();
    }
#endif
}

auto RenderSystem::CreateFactory() -> ComPtr<IDXGIFactory1> {
    auto factory = ComPtr<IDXGIFactory1>{ nullptr };
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
    return factory;
}

auto RenderSystem::CreateDevice(IDXGIFactory1 * factory) -> ComPtr<ID3D12Device> {
    ComPtr<ID3D12Device> device;

    auto adapter = ComPtr<IDXGIAdapter1>{ nullptr };
    for (auto i = 0u; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i) {
        auto description = DXGI_ADAPTER_DESC1{};
        adapter->GetDesc1(&description);
        if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
            break;
        }
    }
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    return device;
}

auto RenderSystem::CreateCommandQueue(ID3D12Device * device) -> ComPtr<ID3D12CommandQueue> {
    auto commandQueue = ComPtr<ID3D12CommandQueue>{ nullptr };
    auto commandQueueDesc = D3D12_COMMAND_QUEUE_DESC{};
    //commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    //commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
    return commandQueue;
}

auto RenderSystem::CreateSrvHeap(ID3D12Device * device, unsigned int descriptorCount) -> ComPtr<ID3D12DescriptorHeap> {
    auto ret = ComPtr<ID3D12DescriptorHeap>{ nullptr };
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = descriptorCount;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&ret)));
    return ret;
}

auto RenderSystem::CreateRootSignature(ID3D12Device * device) -> ComPtr<ID3D12RootSignature> {
    auto ret = ComPtr<ID3D12RootSignature>{ nullptr };

    CD3DX12_DESCRIPTOR_RANGE ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    //rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&ret)));
    return ret;
}

auto RenderSystem::CreatePso(ID3D12Device * device, ID3D12RootSignature * rootSignature) -> ComPtr<ID3D12PipelineState> {
    auto ret = ComPtr<ID3D12PipelineState>{ nullptr };

    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif
    ThrowIfFailed(D3DCompileFromFile(L"shader/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"shader/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

    // Define the vertex input layout.
    auto const inputElementDescs = array<D3D12_INPUT_ELEMENT_DESC, 3> {
        D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = rootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&ret)));
    return ret;
}

auto RenderSystem::CreateCommandList(ID3D12Device * device, ID3D12PipelineState * pso, ID3D12CommandAllocator * allocator) -> ComPtr<ID3D12GraphicsCommandList> {
    auto ret = ComPtr<ID3D12GraphicsCommandList>{ nullptr };
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, pso, IID_PPV_ARGS(&ret)));
    return ret;
}

auto RenderSystem::LoadBegin() -> void {
    _rootSignature = CreateRootSignature(_device.Get());
    _defaultPso = CreatePso(_device.Get(), _rootSignature.Get());
    auto allocator = _frameManager.GetCurrentFrameResource()->GetCommandAllocator();
    _commandList = CreateCommandList(_device.Get(), _defaultPso.Get(), allocator);

    // Define the geometry for a triangle.
    //auto vertices = std::array<float, 21>{
    //    0.0f, 0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    //        0.25f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    //        -0.25f, -0.25f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
    //};
    //Vertex triangleVertices[] =
    //{
    //    { { 0.0f, 0.25f, 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
    //    { { 0.25f, -0.25f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
    //    { { -0.25f, -0.25f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
    //};
    //_vertexBuffer = _resourceManager.CreateVertexBuffer(_commandList.Get(), vertices.data(), vertices.size() * sizeof(float), sizeof(Vertex));

}

auto RenderSystem::LoadMeshes(std::vector<std::unique_ptr<core::Mesh>> const & meshes) -> void {
    _resourceManager.LoadMeshes(_commandList.Get(), meshes, sizeof(core::Vertex));
}

auto RenderSystem::LoadEnd() -> void {
    ThrowIfFailed(_commandList->Close());
    auto list = static_cast<ID3D12CommandList *>(_commandList.Get());
    _commandQueue->ExecuteCommandLists(1, &list);

    // wait until assets have been uploaded to the GPU
    _fence.Sync();
}

}