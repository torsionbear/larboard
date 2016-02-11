#include "ResourceManager.h"

#include <array>
#include <vector>
#include <memory>

#include "d3dx12.h"
#include <D3Dcompiler.h>

#include "Common.h"

#include "core/Vertex.h"

using std::array;
using std::vector;
using std::unique_ptr;

namespace d3d12RenderSystem {

auto ResourceManager::Init(ID3D12Device * device, IDXGIFactory1 * factory, FencedCommandQueue * fencedCommandQueue, unsigned int width, unsigned int height, HWND hwnd) -> void {
    _device = device;
    _frameResourceContainer.Init(device);
    _fencedCommandQueue = fencedCommandQueue;
    _swapChainRenderTargets.Init(factory, device, _fencedCommandQueue->GetCommandQueue(), width, height, hwnd);
    _uploadHeap.Init(65536, device, _fencedCommandQueue);

    _rootSignature = CreateRootSignature();
    _defaultPso = CreatePso(_rootSignature.Get());
    auto allocator = _frameResourceContainer.GetCurrent().GetCommandAllocator();
    _commandList = CreateCommandList(_defaultPso.Get(), allocator);
}

auto ResourceManager::PrepareResource() -> void {
    auto & frameResource = _frameResourceContainer.Switch();
    _fencedCommandQueue->Sync(frameResource.GetFenceValue());
    frameResource.Reset();
    frameResource.SetFenceValue(_fencedCommandQueue->GetFenceValue());

    // reset command list immediately after submission to reuse the allocated memory.
    ThrowIfFailed(_commandList->Reset(frameResource.GetCommandAllocator(), _defaultPso.Get()));
}

auto ResourceManager::LoadEnd() -> void {
    ThrowIfFailed(_commandList->Close());
    _fencedCommandQueue->ExecuteCommandList(_commandList.Get(), 1);
    _fencedCommandQueue->SyncLatest();
}

auto ResourceManager::CreatePso(ID3D12RootSignature * rootSignature) -> ComPtr<ID3D12PipelineState> {
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
    auto rasterizer_desc = D3D12_RASTERIZER_DESC{

    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs.data(), inputElementDescs.size() };
    psoDesc.pRootSignature = rootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(
        D3D12_FILL_MODE_SOLID,
        D3D12_CULL_MODE_BACK,
        TRUE,   // make ccw front here
        D3D12_DEFAULT_DEPTH_BIAS,
        D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
        TRUE,
        FALSE,
        FALSE,
        0,
        D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&ret)));
    return ret;
}

auto ResourceManager::CreateRootSignature() -> ComPtr<ID3D12RootSignature> {
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
    ThrowIfFailed(_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&ret)));
    return ret;
}

auto ResourceManager::CreateCommandList(ID3D12PipelineState * pso, ID3D12CommandAllocator * allocator) -> ComPtr<ID3D12GraphicsCommandList> {
    auto ret = ComPtr<ID3D12GraphicsCommandList>{ nullptr };
    ThrowIfFailed(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, pso, IID_PPV_ARGS(&ret)));
    return ret;
}

auto ResourceManager::LoadMeshes(vector<unique_ptr<core::Mesh>> const& meshes, unsigned int stride) -> void {
    auto vertexData = vector<core::Vertex>();
    auto indexData = vector<unsigned int>();
    auto const vertexIndexBufferIndex = _vertexIndexBuffer.size();
    _vertexIndexBuffer.emplace_back();
    auto & vertexIndexBuffer = _vertexIndexBuffer.back();
    for (auto & mesh : meshes) {
        mesh->_renderDataId = _meshData.size();
        _meshData.push_back(MeshData{ 
            vertexIndexBufferIndex,
            mesh->GetIndex().size(),
            indexData.size() * sizeof(unsigned int),
            static_cast<int>(vertexData.size()),
        });
        vertexData.insert(vertexData.end(), mesh->GetVertex().cbegin(), mesh->GetVertex().cend());
        indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
    }

    auto dataBlocks = std::array<DataBlock, 2>{
        DataBlock{ 0, vertexData.size() * stride, sizeof(float), vertexData.data() },
            DataBlock{ 0, indexData.size() * sizeof(unsigned int), sizeof(float), indexData.data() },
    };
    auto const& memoryBlock = _uploadHeap.AllocateDataBlocks(dataBlocks.data(), dataBlocks.size());

    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(memoryBlock._size),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_vertexIndexHeap)));
    vertexIndexBuffer.vbv = D3D12_VERTEX_BUFFER_VIEW{ _vertexIndexHeap->GetGPUVirtualAddress() + dataBlocks[0].offset, dataBlocks[0].size, stride };
    vertexIndexBuffer.ibv = D3D12_INDEX_BUFFER_VIEW{ _vertexIndexHeap->GetGPUVirtualAddress() + dataBlocks[1].offset, dataBlocks[1].size, DXGI_FORMAT_R32_UINT };

    _uploadHeap.UploadDataBlocks(_commandList.Get(), memoryBlock, _vertexIndexHeap.Get());
}

}