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
    _uploadHeap.Init(16*1024*1024, device, _fencedCommandQueue);

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

auto ResourceManager::LoadBegin(unsigned int depthStencilCount, unsigned int cameraCount, unsigned int meshCount, unsigned int modelCount, unsigned int textureCount) -> void {
    AllocDsvDescriptorHeap(depthStencilCount);
    AllocCbvSrvDescriptorHeap(cameraCount + modelCount + textureCount);
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
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
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

    auto ranges = array<CD3DX12_DESCRIPTOR_RANGE, 3>{};
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, RegisterConvention::Diffuse);	// diffuse texture
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, RegisterConvention::Transform);  // transform
    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, RegisterConvention::Camera);  // camera

    auto rootParameters = array<CD3DX12_ROOT_PARAMETER, 3>{};
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_VERTEX);

    auto sampler = D3D12_STATIC_SAMPLER_DESC{};
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
    sampler.ShaderRegister = RegisterConvention::StaticSampler;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(rootParameters.size(), rootParameters.data(), 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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

auto ResourceManager::CreateDepthStencil(unsigned int width, unsigned int height) -> void {
    auto & bufferInfo = _dsvHeap.GetBufferInfo();

    CD3DX12_RESOURCE_DESC shadowTextureDesc(
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        0,
        width,
        height,
        1,
        1,
        DXGI_FORMAT_D32_FLOAT,
        1,
        0,
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

    D3D12_CLEAR_VALUE clearValue;	// tell the runtime at resource creation the desired clear value for better performance.
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    _defaultBuffers.emplace_back();
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &shadowTextureDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&_defaultBuffers.back())));
    auto buffer = _defaultBuffers.back().Get();

    // Create the depth stencil view.
    _device->CreateDepthStencilView(buffer, nullptr, bufferInfo._cpuHandle);
    _depthStencilBufferInfos.push_back(bufferInfo);
}

auto ResourceManager::LoadMeshes(core::Mesh ** meshes, unsigned int count, unsigned int stride) -> void {
    auto vertexBufferSize = 0u;
    auto indexBufferSize = 0u;
    for (auto i = 0u; i < count; ++i) {
        vertexBufferSize += meshes[i]->GetVertex().size() * stride;
        indexBufferSize += meshes[i]->GetIndex().size() * sizeof(unsigned int);
    }
    _defaultBuffers.emplace_back();
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_defaultBuffers.back())));
    auto vertexBuffer = _defaultBuffers.back().Get();
    auto const vbv = D3D12_VERTEX_BUFFER_VIEW{ vertexBuffer->GetGPUVirtualAddress(), vertexBufferSize, stride };

    _defaultBuffers.emplace_back();
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_defaultBuffers.back())));
    auto indexBuffer = _defaultBuffers.back().Get();
    auto const ibv = D3D12_INDEX_BUFFER_VIEW{ indexBuffer->GetGPUVirtualAddress(), indexBufferSize, DXGI_FORMAT_R32_UINT };

    auto vertexData = vector<core::Vertex>();
    auto indexData = vector<unsigned int>();
    for (auto i = 0u; i < count; ++i) {
        auto mesh = meshes[i];
        mesh->_renderDataId = _meshDataInfos.size();
        _meshDataInfos.push_back(MeshDataInfo{
            vbv,
            ibv,
            mesh->GetIndex().size(),
            indexData.size(),
            static_cast<int>(vertexData.size()),
        });
        vertexData.insert(vertexData.end(), mesh->GetVertex().cbegin(), mesh->GetVertex().cend());
        indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
    }
    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), vertexBuffer, vertexBufferSize, sizeof(float), vertexData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), indexBuffer, indexBufferSize, sizeof(float), indexData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
}

auto ResourceManager::LoadModels(core::Model ** models, unsigned int count) -> void {
    // create resource
    _defaultBuffers.emplace_back();
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(TransformData)),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_defaultBuffers.back())));
    auto buffer = _defaultBuffers.back().Get();
    // aggregate data, populate _transformBufferInfos and set model._renderDataId
    auto transformData = vector<TransformData>{};
    for (auto i = 0u; i < count; ++i) {
        auto & model = models[i];
        model->_renderDataId = _transformBufferInfos.size();
        auto bufferInfo = _cbvSrvHeap.GetBufferInfo();
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { buffer->GetGPUVirtualAddress() + i * sizeof(TransformData), sizeof(TransformData) };
        _device->CreateConstantBufferView(&cbvDesc, bufferInfo._cpuHandle);
        _transformBufferInfos.push_back(bufferInfo);
        transformData.push_back(TransformData{
            model->GetTransform(),
            model->GetTransform(),
        });
    }

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), buffer, sizeof(TransformData) * count, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, transformData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

auto ResourceManager::LoadCamera(core::Camera * cameras, unsigned int count) -> void {
    _uploadBuffers.emplace_back();
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(CameraData)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_uploadBuffers.back())));
    auto buffer = _uploadBuffers.back().Get();
    // Map the constant buffers and cache their heap pointers.
    uint8 * mappedPtr = nullptr;
    CD3DX12_RANGE readRange(0, 0);		// no intend to read from this resource on the CPU.
    ThrowIfFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedPtr)));

    auto cameraData = vector<CameraData>{};
    for (auto i = 0u; i < count; ++i) {
        auto & camera = cameras[i];
        camera._renderDataId = _cameraBufferInfos.size();
        auto bufferInfo = _cbvSrvHeap.GetBufferInfo();
        bufferInfo._mappedDataPtr = mappedPtr + i * sizeof(CameraData);
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { buffer->GetGPUVirtualAddress() + i * sizeof(CameraData), sizeof(CameraData) };
        _device->CreateConstantBufferView(&cbvDesc, bufferInfo._cpuHandle);
        _cameraBufferInfos.push_back(bufferInfo);
        cameraData.push_back(CameraData{
            camera.GetRigidBodyMatrixInverse(),
            camera.GetProjectTransform(),
            camera.GetTransform(),
            camera.GetPosition(),
        });
    }
    memcpy(mappedPtr, cameraData.data(), cameraData.size());
}

auto ResourceManager::LoadTexture(core::Texture * texture) -> void {
    auto const mipmapLevel = 1u; // dx12 does not support auto mipmap generation :(
    auto const format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    CD3DX12_RESOURCE_DESC desc(
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        0,
        texture->GetWidth(),
        texture->GetHeight(),
        1,
        mipmapLevel,
        format,
        1,
        0,
        D3D12_TEXTURE_LAYOUT_UNKNOWN,
        D3D12_RESOURCE_FLAG_NONE);

    _defaultBuffers.emplace_back();
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_defaultBuffers.back())));
    auto buffer = _defaultBuffers.back().Get();
    D3D12_SUBRESOURCE_DATA textureData = { texture->GetData().data(), texture->GetWidth() * sizeof(core::Vector4f), texture->GetData().size() * sizeof(core::Vector4f) };
    _uploadHeap.UploadSubresources(_commandList.Get(), buffer, 0, 1, &textureData);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    auto bufferInfo = _cbvSrvHeap.GetBufferInfo();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = format;
    srvDesc.Texture2D.MipLevels = mipmapLevel;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    _device->CreateShaderResourceView(buffer, &srvDesc, bufferInfo._cpuHandle);
    texture->_renderDataId = _textureBufferInfos.size();
    _textureBufferInfos.push_back(bufferInfo);
}

auto ResourceManager::UpdateCamera(core::Camera const & camera) -> void {
    auto const& bufferInfo = _cameraBufferInfos[camera._renderDataId];
    auto cameraData = CameraData{
        camera.GetRigidBodyMatrixInverse(),
        camera.GetProjectTransform(),
        camera.GetTransform(),
        camera.GetPosition(),
    };
    memcpy(bufferInfo._mappedDataPtr, &cameraData, sizeof(cameraData));
}

}
