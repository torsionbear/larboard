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

auto ResourceManager::LoadBegin() -> void {
    // create null descriptor for the 1st one in _cbvSrvHeap
    D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
    nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    nullSrvDesc.Texture2D.MipLevels = 1;
    nullSrvDesc.Texture2D.MostDetailedMip = 0;
    nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(nullptr);
    _nullDescriptorInfo.push_back(descriptorInfo);
    _device->CreateShaderResourceView(nullptr, &nullSrvDesc, descriptorInfo._cpuHandle);
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
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, SrvRegisterConvention::SrvAll1);
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, -1, SrvRegisterConvention::SrvPsArray);
    ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, CbvRegisterConvention::Transform);
    ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, CbvRegisterConvention::Material);
    ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, CbvRegisterConvention::Camera);
    ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, CbvRegisterConvention::Light);
    ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, CbvRegisterConvention::CbvAll);
    ranges[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, CbvRegisterConvention::Ps2);

    auto rootParameters = array<CD3DX12_ROOT_PARAMETER, 9>{};
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[3].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[5].InitAsDescriptorTable(1, &ranges[5], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[6].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[7].InitAsDescriptorTable(1, &ranges[7], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[8].InitAsConstants(TextureIndex::count, CbvRegisterConvention::TextureIndex);

    auto samplers = std::array<D3D12_STATIC_SAMPLER_DESC, 2>{};
    samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplers[0].MipLODBias = 0;
    samplers[0].MaxAnisotropy = 0;
    samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplers[0].MinLOD = 0.0f;
    samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    samplers[0].ShaderRegister = SamplerRegisterConvention::sampler0;
    samplers[0].RegisterSpace = 0;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    samplers[1] = samplers[0];
    samplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    samplers[1].ShaderRegister = SamplerRegisterConvention::sampler1;
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(rootParameters.size(), rootParameters.data(), samplers.size(), samplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

auto ResourceManager::CreateRenderTarget(DXGI_FORMAT format, unsigned int width, unsigned int height, uint8 size, DescriptorInfo * srv) -> DescriptorInfo {
    auto const mipmapLevel = 1u; // dx12 does not support auto mipmap generation :(

    auto desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, size, mipmapLevel, 1u, 0u, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
    auto clearValue = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f, };
    auto buffer = CreateCommittedResource(&desc, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_HEAP_TYPE_DEFAULT, &CD3DX12_CLEAR_VALUE(format, clearValue.data()));
    if (srv != nullptr) {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.Texture2D.MipLevels = mipmapLevel;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        *srv = _cbvSrvHeap.GetDescriptorInfo(buffer);
        _device->CreateShaderResourceView(buffer, &srvDesc, srv->_cpuHandle);
    }

    auto descriptorInfo = _rtvHeap.GetDescriptorInfo(buffer);
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Format = format;
    rtvDesc.Texture2DArray.ArraySize = size;
    rtvDesc.Texture2DArray.FirstArraySlice = 0u;
    rtvDesc.Texture2DArray.MipSlice = 0u;
    rtvDesc.Texture2DArray.PlaneSlice = 0u;

    _device->CreateRenderTargetView(buffer, &rtvDesc, descriptorInfo._cpuHandle);

    return descriptorInfo;
}

auto ResourceManager::CreateDepthStencil(unsigned int width, unsigned int height, DescriptorInfo * srv) -> DescriptorInfo {
    auto const flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;// | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    auto desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, width, height, 1u, 1u, 1u, 0u, flags);
    auto clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0u); // cannot use R32_TYPELESS here for dsv. Use D32_FLOAT instead.
    auto buffer = CreateCommittedResource(&desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_HEAP_TYPE_DEFAULT, &clearValue);

    if (srv != nullptr) {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // DXGI_FORMAT_D32_FLOAT cannot be used for srv
        srvDesc.Texture2D.MipLevels = 1u;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        *srv = _cbvSrvHeap.GetDescriptorInfo(buffer);
        _device->CreateShaderResourceView(buffer, &srvDesc, srv->_cpuHandle);
    }
    // Create the depth stencil view.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    auto & descriptorInfo = _dsvHeap.GetDescriptorInfo(buffer);
    _device->CreateDepthStencilView(buffer, &dsvDesc, descriptorInfo._cpuHandle);
    return descriptorInfo;
}

auto ResourceManager::LoadMeshes(core::Mesh ** meshes, unsigned int count, unsigned int stride) -> void {
    if (meshes == nullptr || count == 0) {
        return;
    }
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
    if (models == nullptr || count == 0) {
        return;
    }
    // create resource
    auto buffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(TransformData)), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    // aggregate data, populate _transformDescriptorInfos and set model._renderDataId
    auto transformData = vector<TransformData>{};
    for (auto i = 0u; i < count; ++i) {
        auto & model = models[i];
        model->_renderDataId = _transformDescriptorInfos.size();
        auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(buffer);
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
    if (cameras == nullptr || count == 0) {
        return;
    }
    auto buffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(CameraData)), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
    // Map the constant buffers and cache their heap pointers.
    uint8 * mappedPtr = nullptr;
    CD3DX12_RANGE readRange(0, 0);		// no intend to read from this resource on the CPU.
    ThrowIfFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedPtr)));

    auto cameraData = vector<CameraData>{};
    for (auto i = 0u; i < count; ++i) {
        auto & camera = cameras[i];
        camera.SetRenderDataId(_cameraDescriptorInfos.size());
        auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(buffer);
        descriptorInfo._mappedDataPtr = mappedPtr + i * sizeof(CameraData);
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { buffer->GetGPUVirtualAddress() + i * sizeof(CameraData), sizeof(CameraData) };
        _device->CreateConstantBufferView(&cbvDesc, descriptorInfo._cpuHandle);
        _cameraDescriptorInfos.push_back(descriptorInfo);
        cameraData.push_back(CameraData{
            camera.GetRigidBodyMatrixInverse(),
            camera.GetProjectTransformDx(),
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

    _lightDescriptorInfo = _cbvSrvHeap.GetDescriptorInfo(buffer);
    auto desc = D3D12_CONSTANT_BUFFER_VIEW_DESC{ buffer->GetGPUVirtualAddress(), sizeof(LightData) };
    _device->CreateConstantBufferView(&desc, _lightDescriptorInfo._cpuHandle);

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), buffer, sizeof(LightData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, &lightData);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

auto ResourceManager::LoadShadowCastingLight(core::DirectionalLight ** directionalLights, unsigned int directionalLightCount) -> void {
    if (directionalLightCount == 0) {
        return;
    }
    auto buffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(directionalLightCount * sizeof(CameraData)), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
    // Map the constant buffers and cache their heap pointers.
    uint8 * mappedPtr = nullptr;
    CD3DX12_RANGE readRange(0, 0);		// no intend to read from this resource on the CPU.
    ThrowIfFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedPtr)));

    auto cameraData = vector<CameraData>{};
    for (auto i = 0u; i < directionalLightCount; ++i) {
        auto & directionalLight = directionalLights[i];
        directionalLight->SetRenderDataId(_cameraDescriptorInfos.size());
        auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(buffer);
        descriptorInfo._mappedDataPtr = mappedPtr + i * sizeof(CameraData);
        auto cbvDesc = D3D12_CONSTANT_BUFFER_VIEW_DESC{ buffer->GetGPUVirtualAddress() + i * sizeof(CameraData), sizeof(CameraData) };
        _device->CreateConstantBufferView(&cbvDesc, descriptorInfo._cpuHandle);
        _cameraDescriptorInfos.push_back(descriptorInfo);
        cameraData.push_back(CameraData{
            directionalLight->GetRigidBodyMatrixInverse(),
            directionalLight->GetProjectTransformDx(),
            directionalLight->GetTransform(),
            directionalLight->GetPosition(),
        });
    }
    memcpy(mappedPtr, cameraData.data(), cameraData.size());
}

auto ResourceManager::LoadMaterials(core::Material ** materials, unsigned int count) -> void {
    if (materials == nullptr || count == 0) {
        return;
    }
    auto buffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(MaterialData)), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);

    auto materialData = vector<MaterialData>{};
    for (auto i = 0u; i < count; ++i) {
        auto & material = materials[i];
        material->_renderDataId = _materialDescriptorInfos.size();
        auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(buffer);
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
            material->GetTransparency(),
        });
    }

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), buffer, sizeof(MaterialData) * count, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, materialData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

auto ResourceManager::LoadDdsTexture(core::Texture ** texture, unsigned int count) -> void {
    for (auto i = 0u; i < count; ++i) {
        // change filename's extension to .dds
        auto filename = texture[i]->GetFilename();
        filename.erase(filename.find_last_of('.'));
        filename += ".dds";
        texture[i]->_renderDataId = LoadDdsTexture(filename);
    }
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

    // 3. upload texture
    auto desc = uploadBuffer->GetDesc();
    auto buffer = CreateCommittedResource(&desc, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    _commandList->CopyResource(buffer, uploadBuffer);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    // 4. create srv
    auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(buffer);
    _device->CreateShaderResourceView(buffer, &srvDesc, descriptorInfo._cpuHandle);
    auto ret = _textureDescriptorInfos.size();
    _textureDescriptorInfos.push_back(descriptorInfo);
    return ret;
}

auto ResourceManager::LoadTexture(core::Texture * texture) -> void {
    auto const mipmapLevel = 1u; // dx12 does not support auto mipmap generation :(
    auto const format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    //
    //auto desc = CD3DX12_RESOURCE_DESC::Tex2D(format, texture->GetWidth(), texture->GetHeight(), 1u, mipmapLevel);
    //auto buffer = CreateCommittedResource(&desc, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    //D3D12_SUBRESOURCE_DATA textureData = { texture->GetData().data(), texture->GetWidth() * sizeof(core::Vector4f), texture->GetData().size() * sizeof(core::Vector4f) };
    //_uploadHeap.UploadSubresources(_commandList.Get(), buffer, 0, 1, &textureData);
    //_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    //
    //D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    //srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    //srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    //srvDesc.Format = format;
    //srvDesc.Texture2D.MipLevels = mipmapLevel;
    //srvDesc.Texture2D.MostDetailedMip = 0;
    //srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    //auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(buffer);
    //_device->CreateShaderResourceView(buffer, &srvDesc, descriptorInfo._cpuHandle);

    auto descriptorInfo = CreateTexture2d(format, texture->GetWidth(), texture->GetHeight(), texture->GetData().data(), texture->GetData().size(), sizeof(core::Vector4f));
    texture->_renderDataId = _textureDescriptorInfos.size();
    _textureDescriptorInfos.push_back(descriptorInfo);
}
auto ResourceManager::CreateTexture2d(DXGI_FORMAT format, uint64 width, uint32 height, void const* data, uint32 size, uint8 stride) -> DescriptorInfo {
    auto const mipmapLevel = 1u; // dx12 does not support auto mipmap generation :(
    auto desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1u, mipmapLevel);
    auto buffer = CreateCommittedResource(&desc, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    D3D12_SUBRESOURCE_DATA textureData = { data, width * stride, size * stride };
    _uploadHeap.UploadSubresources(_commandList.Get(), buffer, 0, 1, &textureData);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = format;
    srvDesc.Texture2D.MipLevels = mipmapLevel;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(buffer);
    _device->CreateShaderResourceView(buffer, &srvDesc, descriptorInfo._cpuHandle);
    return descriptorInfo;

}

auto ResourceManager::UploadConstantBufferData(unsigned int size, void const* data, ID3D12Resource * dest) -> DescriptorInfo {
    if (dest == nullptr) {
        dest = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    } else {
        _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dest, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST));
    }
    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), dest, size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, data);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    auto descriptorInfo = _cbvSrvHeap.GetDescriptorInfo(dest);
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { dest->GetGPUVirtualAddress(), size };
    _device->CreateConstantBufferView(&cbvDesc, descriptorInfo._cpuHandle);
    return descriptorInfo;
}

auto ResourceManager::UploadVertexData(unsigned int size, unsigned int stride, void const* data, ID3D12Resource ** dest) -> D3D12_VERTEX_BUFFER_VIEW {
    auto resource = static_cast<ID3D12Resource *>(nullptr);
    if (dest != nullptr && *dest != nullptr) {
        _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*dest, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST));
        resource = *dest;
    } else {
        resource = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
        if (dest != nullptr) {
            *dest = resource;
        }
    }
    if (data != nullptr) {
        _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), resource, size, sizeof(float), data);
        _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
    }
    return D3D12_VERTEX_BUFFER_VIEW{ resource->GetGPUVirtualAddress(), size, stride };
}

auto ResourceManager::UploadIndexData(unsigned int size, void const* data) -> D3D12_INDEX_BUFFER_VIEW {
    auto indexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    if (data != nullptr) {
        _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), indexBuffer, size, sizeof(float), data);
        _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
    }
    return D3D12_INDEX_BUFFER_VIEW{ indexBuffer->GetGPUVirtualAddress(), size, DXGI_FORMAT_R32_UINT };
}

auto ResourceManager::LoadSkyBox(core::SkyBox * skyBox) -> void {
    if (skyBox == nullptr) {
        return;
    }
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
    auto vertexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    auto const vbv = D3D12_VERTEX_BUFFER_VIEW{ vertexBuffer->GetGPUVirtualAddress(), vertexBufferSize, sizeof(core::Vector3f) };

    auto const indexBufferSize = indexData.size() * sizeof(unsigned int);
    auto indexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    auto const ibv = D3D12_INDEX_BUFFER_VIEW{ indexBuffer->GetGPUVirtualAddress(), indexBufferSize, DXGI_FORMAT_R32_UINT };

    _skyBoxMeshInfo = MeshDataInfo{ vbv, ibv, indexData.size(), 0u, 0u, };

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), vertexBuffer, vertexBufferSize, sizeof(float), vertexData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    _uploadHeap.AllocateAndUploadDataBlock(_commandList.Get(), indexBuffer, indexBufferSize, sizeof(float), indexData.data());
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

    skyBox->SetRenderDataId(LoadDdsTexture(skyBox->GetFilename()));
}

auto ResourceManager::LoadTerrain(core::Terrain * terrain) -> void {
    terrain->GetDiffuseMap()->_renderDataId = LoadDdsTexture(terrain->GetDiffuseMapFilename());
    terrain->GetHeightMap()->_renderDataId = LoadDdsTexture(terrain->GetHeightMapFilename());
    
    auto const tileSize = terrain->GetTileSize();
    auto vertexData = std::array<core::Vector2f, 4>{ core::Vector2f{ 0, 0 }, core::Vector2f{ tileSize, 0 }, core::Vector2f{ tileSize, tileSize }, core::Vector2f{ 0, tileSize } };
    auto vertexBufferView = UploadVertexData(vertexData.size() * sizeof(core::Vector2f), sizeof(core::Vector2f), vertexData.data());
    auto indexData = std::array<unsigned int, 6>{0, 1, 2, 0, 2, 3};
    auto indexBufferView = UploadIndexData(indexData.size() * sizeof(unsigned int), indexData.data());

    auto sightDistance = terrain->GetSightDistance();
    auto tileCountUpperBound = static_cast<int>(ceil(sightDistance / tileSize) * 2);
    tileCountUpperBound *= tileCountUpperBound;
    auto specialTiles = terrain->GetSpecialTiles();
    auto instanceData = vector<core::Vector3f>(specialTiles.size() + tileCountUpperBound, core::Vector3f{0, 0, 0});
    auto instanceBufferView = UploadVertexData(instanceData.size() * sizeof(core::Vector3f), sizeof(core::Vector3f), instanceData.data(), &_terrainInstanceDataResource);
    _terrainMeshInfo = MeshDataInfo {
        vertexBufferView,
        indexBufferView,
        indexData.size(),
        0u,
        0,
        instanceBufferView,
        static_cast<unsigned int>(tileCountUpperBound), // should use terrain->GetTileCount() instead for draw call
        specialTiles.size(), // reserve first specialTiles.size() Vector3f{0, 0, 0} for special Tiles
    };
    auto terrainData = TerrainData {
        terrain->GetHeightMapOrigin(),
        terrain->GetHeightMapSize(),
        terrain->GetDiffuseMapOrigin(),
        terrain->GetDiffuseMapSize(),
        terrain->GetTileSize(),
        terrain->GetSightDistance(),        
    };
    _terrainCbv = UploadConstantBufferData(sizeof(TerrainData), &terrainData);

    //special tiles
    LoadMeshes(specialTiles.data(), specialTiles.size(), sizeof(core::Vertex));
}

auto ResourceManager::UpdateViewpoint(core::Viewpoint const * viewpoint) -> void {
    auto const& descriptorInfo = _cameraDescriptorInfos[viewpoint->GetRenderDataId()];
    auto cameraData = CameraData{
        viewpoint->GetRigidBodyMatrixInverse(),
        viewpoint->GetProjectTransformDx(),
        viewpoint->GetTransform(),
        viewpoint->GetPosition(),
    };
    memcpy(descriptorInfo._mappedDataPtr, &cameraData, sizeof(cameraData));
}

auto ResourceManager::UpdateTerrain(core::Terrain * terrain, core::Camera * camera) -> void {
    // update terrain tile coord ubo
    auto const& tileCoord = terrain->GetTileCoordinateWithSpecialTiles(camera);
    UploadVertexData(tileCoord.size() * sizeof(core::Vector3f), sizeof(core::Vector3f), tileCoord.data(), &_terrainInstanceDataResource);
}

auto ResourceManager::CreateBundle(ID3D12PipelineState * pso, ID3D12RootSignature * rootSignature, ID3D12DescriptorHeap *const* descriptorHeaps, unsigned int descriptorHeapCount) -> ComPtr<ID3D12GraphicsCommandList> {
    auto ret = ComPtr<ID3D12GraphicsCommandList>{ nullptr };
    _commandAllocators.emplace_back(nullptr);
    auto & commandAllocator = _commandAllocators.back();
    ThrowIfFailed(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&commandAllocator)));
    ThrowIfFailed(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, commandAllocator.Get(), pso, IID_PPV_ARGS(&ret)));
    // Bundle command lists must explcitly set the root signature before making any changes to root descriptor tables
    ret->SetGraphicsRootSignature(rootSignature);
    // Bundle command lists must explcitly set descriptor heap before setting its handle to root descriptor tables
    ret->SetDescriptorHeaps(descriptorHeapCount, descriptorHeaps);

    return ret;
}

}
