#include "ResourceManager.h"

#include <array>
#include <vector>
#include <memory>

#include "d3dx12.h"
#include <D3Dcompiler.h>

#include "Common.h"
#include "DDSTextureLoader.h"

#include "core/Vertex.h"

using std::array;
using std::vector;
using std::unique_ptr;
using std::string;

namespace d3d12RenderSystem {

auto ResourceManager::CreateDevice(IDXGIFactory1 * factory) -> ComPtr<ID3D12Device> {
    ComPtr<ID3D12Device> device;
    auto adapter = ComPtr<IDXGIAdapter1>{ nullptr };
    for (auto i = 0u; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i) {
        auto description = DXGI_ADAPTER_DESC1{};
        adapter->GetDesc1(&description);
        //if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        //    continue;
        //}
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
            break;
        }
    }
    ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
    return device;
}

ResourceManager::ResourceManager(IDXGIFactory1 * factory, unsigned int width, unsigned int height, HWND hwnd){
    _device = CreateDevice(factory);
    _frameResourceContainer.Init(_device.Get());
    _fencedCommandQueue.Init(_device.Get());
    _swapChainRenderTargets.Init(factory, _device.Get(), _fencedCommandQueue.GetCommandQueue(), width, height, hwnd);
    _uploadHeap.Init(256*1024*1024, _device.Get(), &_fencedCommandQueue);

    _rootSignature = CreateRootSignature();
    auto allocator = _frameResourceContainer.GetCurrent().GetCommandAllocator();
    _commandList = CreateCommandList(nullptr, allocator);
}

auto ResourceManager::PrepareResource() -> void {
    auto & frameResource = _frameResourceContainer.Switch();
    _fencedCommandQueue.Sync(frameResource.GetFenceValue());
    frameResource.Reset();
    frameResource.SetFenceValue(_fencedCommandQueue.GetFenceValue());

    // reset command list immediately after submission to reuse the allocated memory.
    ThrowIfFailed(_commandList->Reset(frameResource.GetCommandAllocator(), nullptr));
}

auto ResourceManager::LoadBegin(
    unsigned int depthStencilCount,
    unsigned int cameraCount,
    unsigned int meshCount,
    unsigned int modelCount,
    unsigned int textureCount,
    unsigned int materialCount,
    unsigned int skyBoxCount) -> void {
    auto const lightDescriptorCount = 1u;
    auto const nullDescriptorCount = 4u;
    AllocDsvDescriptorHeap(depthStencilCount);
    AllocCbvSrvDescriptorHeap(cameraCount + modelCount + textureCount + materialCount + lightDescriptorCount + nullDescriptorCount + skyBoxCount);

    // create null descriptor
    for (auto i = 0u; i < nullDescriptorCount; ++i) {
        D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
        nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        nullSrvDesc.Texture2D.MipLevels = 1;
        nullSrvDesc.Texture2D.MostDetailedMip = 0;
        nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo();
        _nullDescriptorInfo.push_back(descriptorInfo);
        _device->CreateShaderResourceView(nullptr, &nullSrvDesc, descriptorInfo._cpuHandle);
    }
}

auto ResourceManager::LoadEnd() -> void {
    ThrowIfFailed(_commandList->Close());
    _fencedCommandQueue.ExecuteCommandList(_commandList.Get(), 1);
    _fencedCommandQueue.SyncLatest();
}

auto ResourceManager::CreatePso(D3D12_GRAPHICS_PIPELINE_STATE_DESC const* psoDesc) -> ComPtr<ID3D12PipelineState> {
    auto ret = ComPtr<ID3D12PipelineState>{ nullptr };
    ThrowIfFailed(_device->CreateGraphicsPipelineState(psoDesc, IID_PPV_ARGS(&ret)));
    return ret;
}

auto ResourceManager::CreateRootSignature() -> ComPtr<ID3D12RootSignature> {
    auto ret = ComPtr<ID3D12RootSignature>{ nullptr };

    auto ranges = array<CD3DX12_DESCRIPTOR_RANGE, 8>{};
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, RegisterConvention::DiffuseMap);
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, RegisterConvention::NormalMap);
    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, RegisterConvention::SpecularMap);
    ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, RegisterConvention::EmissiveMap);
    ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, RegisterConvention::Transform);
    ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, RegisterConvention::Material);
    ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, RegisterConvention::Camera);
    ranges[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, RegisterConvention::Light);

    auto rootParameters = array<CD3DX12_ROOT_PARAMETER, 8>{};
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[5].InitAsDescriptorTable(1, &ranges[5], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[6].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[7].InitAsDescriptorTable(1, &ranges[7], D3D12_SHADER_VISIBILITY_PIXEL);

    auto sampler = D3D12_STATIC_SAMPLER_DESC{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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

auto ResourceManager::CreateCommittedResource(
    D3D12_RESOURCE_DESC const * desc,
    D3D12_RESOURCE_STATES resourceState,
    D3D12_HEAP_TYPE heapType,
    D3D12_CLEAR_VALUE * clearValue) -> ID3D12Resource * {
    auto buffers = static_cast<vector<ComPtr<ID3D12Resource>> *>(nullptr);
    switch (heapType) {
    case D3D12_HEAP_TYPE_DEFAULT:
        buffers = &_defaultBuffers;
        break;
    case D3D12_HEAP_TYPE_UPLOAD:
        buffers = &_uploadBuffers;
        break;
    }

    buffers->emplace_back();
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(heapType),
        D3D12_HEAP_FLAG_NONE,
        desc,
        resourceState,
        clearValue,
        IID_PPV_ARGS(&buffers->back())));
    return buffers->back().Get();
}

auto ResourceManager::CompileShader(string const& filename, string const& target) -> ComPtr<ID3DBlob> {
    auto compileFlags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    compileFlags = compileFlags | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> ret;
    ThrowIfFailed(D3DCompileFromFile(StringToWstring(filename).data(), nullptr, nullptr, "main", target.data(), compileFlags, 0, &ret, nullptr));
    return ret;
}

auto ResourceManager::CreateDepthStencil(unsigned int width, unsigned int height) -> void {
    auto const flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    auto desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1u, 1u, 1u, 0u, flags);
    auto clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0u);
    auto buffer = CreateCommittedResource(&desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_HEAP_TYPE_DEFAULT, &clearValue);

    // Create the depth stencil view.
    auto & descriptorInfo = _dsvHeap.GetDescriptorInfo();
    _device->CreateDepthStencilView(buffer, nullptr, descriptorInfo._cpuHandle);
    _depthStencilDescriptorInfos.push_back(descriptorInfo);
}

auto ResourceManager::LoadMeshes(core::Mesh ** meshes, unsigned int count, unsigned int stride) -> void {
    auto vertexBufferSize = 0u;
    auto indexBufferSize = 0u;
    for (auto i = 0u; i < count; ++i) {
        vertexBufferSize += meshes[i]->GetVertex().size() * stride;
        indexBufferSize += meshes[i]->GetIndex().size() * sizeof(unsigned int);
    }
    auto vertexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    auto const vbv = D3D12_VERTEX_BUFFER_VIEW{ vertexBuffer->GetGPUVirtualAddress(), vertexBufferSize, stride };

    auto indexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
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
    auto buffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(TransformData)), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    // aggregate data, populate _transformDescriptorInfos and set model._renderDataId
    auto transformData = vector<TransformData>{};
    for (auto i = 0u; i < count; ++i) {
        auto & model = models[i];
        model->_renderDataId = _transformDescriptorInfos.size();
        auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo();
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { buffer->GetGPUVirtualAddress() + i * sizeof(TransformData), sizeof(TransformData) };
        _device->CreateConstantBufferView(&cbvDesc, descriptorInfo._cpuHandle);
        _transformDescriptorInfos.push_back(descriptorInfo);
        transformData.push_back(TransformData{
            model->GetTransform(),
            model->GetTransform(),
        });
    }

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), buffer, sizeof(TransformData) * count, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, transformData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

auto ResourceManager::LoadCamera(core::Camera * cameras, unsigned int count) -> void {
    auto buffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(CameraData)), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
    // Map the constant buffers and cache their heap pointers.
    uint8 * mappedPtr = nullptr;
    CD3DX12_RANGE readRange(0, 0);		// no intend to read from this resource on the CPU.
    ThrowIfFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedPtr)));

    auto cameraData = vector<CameraData>{};
    for (auto i = 0u; i < count; ++i) {
        auto & camera = cameras[i];
        camera._renderDataId = _cameraDescriptorInfos.size();
        auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo();
        descriptorInfo._mappedDataPtr = mappedPtr + i * sizeof(CameraData);
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { buffer->GetGPUVirtualAddress() + i * sizeof(CameraData), sizeof(CameraData) };
        _device->CreateConstantBufferView(&cbvDesc, descriptorInfo._cpuHandle);
        _cameraDescriptorInfos.push_back(descriptorInfo);
        cameraData.push_back(CameraData{
            camera.GetRigidBodyMatrixInverse(),
            camera.GetProjectTransform(),
            camera.GetTransform(),
            camera.GetPosition(),
        });
    }
    memcpy(mappedPtr, cameraData.data(), cameraData.size());
}

auto ResourceManager::LoadLight(core::AmbientLight ** ambientLights, unsigned int ambientLightCount,
    core::DirectionalLight ** directionalLights, unsigned int directionalLightCount,
    core::PointLight ** pointLights, unsigned int pointLightCount,
    core::SpotLight ** spotLights, unsigned int spotLightCount) -> void {
    auto lightData = LightData{ambientLightCount, directionalLightCount, pointLightCount, spotLightCount};
    for (auto i = 0u; i < ambientLightCount; ++i) {
        lightData.ambientLights[i] = LightData::AmbientLight{ ambientLights[i]->GetColor()};
    }
    for (auto i = 0u; i < directionalLightCount; ++i) {
        lightData.directionalLights[i] = LightData::DirectionalLight{directionalLights[i]->GetColor(), directionalLights[i]->GetDirection()};
    }
    for (auto i = 0u; i < pointLightCount; ++i) {
        lightData.pointLights[i] = LightData::PointLight{ pointLights[i]->GetColor(), pointLights[i]->GetPosition(), pointLights[i]->GetAttenuation() };
    }
    for (auto i = 0u; i < spotLightCount; ++i) {
        lightData.spotLights[i] = LightData::SpotLight{
            spotLights[i]->GetColor(),
            spotLights[i]->GetPosition(),
            spotLights[i]->GetDirection(),
            spotLights[i]->GetAttenuation(),
            core::Vector4f{spotLights[i]->GetBeamWidth(), spotLights[i]->GetCutOffAngle(), 0.0f, 0.0f},
        };
    }

    auto buffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(sizeof(LightData)), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);

    _lightDescriptorInfo = _cbvSrvHeap.GetDescriptorInfo();
    auto desc = D3D12_CONSTANT_BUFFER_VIEW_DESC{ buffer->GetGPUVirtualAddress(), sizeof(LightData) };
    _device->CreateConstantBufferView(&desc, _lightDescriptorInfo._cpuHandle);

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), buffer, sizeof(LightData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, &lightData);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

auto ResourceManager::LoadMaterials(core::Material ** materials, unsigned int count) -> void {
    auto buffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(MaterialData)), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);

    auto materialData = vector<MaterialData>{};
    for (auto i = 0u; i < count; ++i) {
        auto & material = materials[i];
        material->_renderDataId = _materialDescriptorInfos.size();
        auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo();
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { buffer->GetGPUVirtualAddress() + i * sizeof(MaterialData), sizeof(MaterialData) };
        _device->CreateConstantBufferView(&cbvDesc, descriptorInfo._cpuHandle);
        _materialDescriptorInfos.push_back(descriptorInfo);
        materialData.push_back(MaterialData{
            material->GetDiffuse(),
            material->_hasDiffuseMap,
            material->GetEmissive(),
            material->_hasEmissiveMap,
            material->GetSpecular(),
            material->_hasSpecularMap,
            material->GetShininess(),
            material->_hasNormalMap,
        });
    }

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), buffer, sizeof(MaterialData) * count, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, materialData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

// quick & dirty implementation to load dds files
auto ResourceManager::LoadDdsTexture(string const& filename) -> unsigned int {

    // 2. create texture (CreateDDSTextureFromFile() uses upload heap. Need to upload data to default heap later)
    //_uploadBuffers.emplace_back();
    //auto uploadBuffer = _uploadBuffers.back().Get();
    auto uploadBuffer = static_cast<ID3D12Resource *>(nullptr);
    auto srvDesc = D3D12_SHADER_RESOURCE_VIEW_DESC{};
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile(_device.Get(), StringToWstring(filename).data(), &uploadBuffer, &srvDesc));

    _uploadBuffers.emplace_back();
    _uploadBuffers.back().Attach(uploadBuffer);
    //_uploadBuffers.emplace_back(uploadBuffer);
    //uploadBuffer->Release();

    // 3. upload texture
    auto desc = uploadBuffer->GetDesc();
    auto buffer = CreateCommittedResource(&desc, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    _commandList->CopyResource(buffer, uploadBuffer);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // 4. create srv
    auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo();
    _device->CreateShaderResourceView(buffer, &srvDesc, descriptorInfo._cpuHandle);
    auto ret = _textureDescriptorInfos.size();
    _textureDescriptorInfos.push_back(descriptorInfo);
    return ret;
}

auto ResourceManager::LoadTexture(core::Texture * texture) -> void {
    auto const mipmapLevel = 1u; // dx12 does not support auto mipmap generation :(
    auto const format = DXGI_FORMAT_R32G32B32A32_FLOAT;

    auto desc = CD3DX12_RESOURCE_DESC::Tex2D(format, texture->GetWidth(), texture->GetHeight(), 1u, mipmapLevel);
    auto buffer = CreateCommittedResource(&desc, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    D3D12_SUBRESOURCE_DATA textureData = { texture->GetData().data(), texture->GetWidth() * sizeof(core::Vector4f), texture->GetData().size() * sizeof(core::Vector4f) };
    _uploadHeap.UploadSubresources(_commandList.Get(), buffer, 0, 1, &textureData);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo();
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = format;
    srvDesc.Texture2D.MipLevels = mipmapLevel;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    _device->CreateShaderResourceView(buffer, &srvDesc, descriptorInfo._cpuHandle);
    texture->_renderDataId = _textureDescriptorInfos.size();
    _textureDescriptorInfos.push_back(descriptorInfo);
}
auto ResourceManager::LoadSkyBox(core::SkyBox * skybox) -> void {
    auto vertexData = vector<core::Vector3f>{
        core::Vector3f{ -1, 1, -1 },
        core::Vector3f{ -1, -1, -1 },
        core::Vector3f{ 1, -1, -1 },
        core::Vector3f{ 1, 1, -1 },
        core::Vector3f{ -1, 1, 1 },
        core::Vector3f{ -1, -1, 1 },
        core::Vector3f{ 1, -1, 1 },
        core::Vector3f{ 1, 1, 1 } };
    auto indexData = vector<unsigned int>{
        0, 1, 2, 0, 2, 3,
        0, 4, 5, 0, 5, 1,
        0, 3, 7, 0, 7, 4,
        2, 6, 7, 2, 7, 3,
        1, 5, 6, 1, 6, 2,
        7, 6, 5, 7, 5, 4 };

    auto const vertexBufferSize = vertexData.size() * sizeof(core::Vector3f);
    auto const indexBufferSize = indexData.size() * sizeof(unsigned int);
    auto vertexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    auto const vbv = D3D12_VERTEX_BUFFER_VIEW{ vertexBuffer->GetGPUVirtualAddress(), vertexBufferSize, sizeof(core::Vector3f) };

    auto indexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    auto const ibv = D3D12_INDEX_BUFFER_VIEW{ indexBuffer->GetGPUVirtualAddress(), indexBufferSize, DXGI_FORMAT_R32_UINT };
    _skyBoxMeshInfo = MeshDataInfo{ vbv, ibv, indexData.size(), 0u, 0u, };

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), vertexBuffer, vertexBufferSize, sizeof(float), vertexData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), indexBuffer, indexBufferSize, sizeof(float), indexData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

    skybox->SetRenderDataId(LoadDdsTexture(skybox->GetFilename()));
}

auto ResourceManager::UpdateCamera(core::Camera const & camera) -> void {
    auto const& descriptorInfo = _cameraDescriptorInfos[camera._renderDataId];
    auto cameraData = CameraData{
        camera.GetRigidBodyMatrixInverse(),
        camera.GetProjectTransform(),
        camera.GetTransform(),
        camera.GetPosition(),
    };
    memcpy(descriptorInfo._mappedDataPtr, &cameraData, sizeof(cameraData));
}

}
