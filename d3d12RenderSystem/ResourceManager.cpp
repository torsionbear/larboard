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

auto ResourceManager::Init(ID3D12Device * device, ID3D12CommandQueue * commandQueue, IDXGIFactory1 * factory, UINT width, UINT height, HWND hwnd) -> void {
    _device = device;
    _commandQueue = commandQueue;
    _swapChainRenderTargets.Init(factory, _device, _commandQueue, width, height, hwnd);

    _fence.Init(_device, _commandQueue);
    auto frameResources = std::vector<FrameResource>(2);
    for (auto & frameResource : frameResources) {
        frameResource.Init(_device);
    }
    _frameManager.Init(&_fence, move(frameResources));
}

auto ResourceManager::FrameBegin() -> void {
    _frameManager.FrameBegin();
    // reset command list immediately after submission to reuse the allocated memory.
    ThrowIfFailed(_commandList->Reset(_frameManager.GetCurrentFrameResource()->GetCommandAllocator(), _defaultPso.Get()));
}

auto ResourceManager::FrameEnd() -> void {
    _frameManager.FrameEnd();
}

auto ResourceManager::LoadBegin() -> void {
    _rootSignature = CreateRootSignature();
    _defaultPso = CreatePso(_rootSignature.Get());
    auto allocator = _frameManager.GetCurrentFrameResource()->GetCommandAllocator();
    _commandList = CreateCommandList(_defaultPso.Get(), allocator);
}

auto ResourceManager::LoadEnd() -> void {
    ThrowIfFailed(_commandList->Close());
    auto list = static_cast<ID3D12CommandList *>(_commandList.Get());
    _commandQueue->ExecuteCommandLists(1, &list);

    // wait until assets have been uploaded to the GPU
    _fence.Sync();
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
    auto vertexDataSize = 0u;
    auto indexDataSize = 0u;
    for (auto const& mesh : meshes) {
        vertexDataSize += mesh->GetVertex().size() * stride;
        indexDataSize += mesh->GetIndex().size() * sizeof(unsigned int);
    }
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_vertexHeap)));    
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_uploadHeap)));
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexDataSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&_indexHeap)));
    ThrowIfFailed(_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexDataSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&_uploadHeap2)));

    auto vbv = D3D12_VERTEX_BUFFER_VIEW{ _vertexHeap->GetGPUVirtualAddress(), vertexDataSize, stride };
    auto ibv = D3D12_INDEX_BUFFER_VIEW{ _indexHeap->GetGPUVirtualAddress(), indexDataSize, DXGI_FORMAT_R32_UINT };
    for (auto & mesh : meshes) {
        mesh->_renderDataId = _meshData.size();
        _meshData.push_back(MeshData{
            vbv,
            ibv,
            mesh->GetIndex().size(),
            indexData.size() * sizeof(unsigned int),
            static_cast<int>(vertexData.size()),
        });
        vertexData.insert(vertexData.end(), mesh->GetVertex().cbegin(), mesh->GetVertex().cend());
        indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
    }
    // Copy data to the upload heap and then schedule a copy 
    // from the upload heap to the vertex buffer.
    auto const vertexDataSize_long= static_cast<LONG_PTR>(vertexDataSize);
    auto vertexSubresourceData = D3D12_SUBRESOURCE_DATA{ vertexData.data(), vertexDataSize_long, vertexDataSize_long };
    UpdateSubresources<1>(_commandList.Get(), _vertexHeap.Get(), _uploadHeap.Get(), 0, 0, 1, &vertexSubresourceData);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_vertexHeap.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    auto const indexDataSize_long = static_cast<LONG_PTR>(indexDataSize);
    auto indexSubresourceData = D3D12_SUBRESOURCE_DATA{ indexData.data(), indexDataSize_long, indexDataSize_long };
    UpdateSubresources<1>(_commandList.Get(), _indexHeap.Get(), _uploadHeap2.Get(), 0, 0, 1, &indexSubresourceData);
    _commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_indexHeap.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
}

/*
auto ResourceManager::LoadMeshes(vector<unique_ptr<Mesh>> const& meshes) -> void {
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();

	glGenVertexArrays(1, &vertexBuffer._vao);
	glBindVertexArray(vertexBuffer._vao);

	glGenBuffers(1, &vertexBuffer._vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);

	glGenBuffers(1, &vertexBuffer._veo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);

	auto vertexData = vector<Vertex>();
	auto indexData = vector<unsigned int>();
	for (auto const& mesh : meshes) {
        mesh->SetRenderData(Mesh::RenderData{
            vertexBuffer._vao,
            indexData.size() * sizeof(unsigned int),
            static_cast<GLint>(vertexData.size()),
        });
		vertexData.insert(vertexData.end(), mesh->GetVertex().cbegin(), mesh->GetVertex().cend());
		indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
	}
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(Vector3f)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(Vector3f)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

auto ResourceManager::LoadSkyBoxMesh(SkyBox * skyBox) -> void {
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();

    auto const vertexData = vector<Vector3f>{
        Vector3f{ -1, 1, -1 },
        Vector3f{ -1, -1, -1 },
        Vector3f{ 1, -1, -1 },
        Vector3f{ 1, 1, -1 },
        Vector3f{ -1, 1, 1 },
        Vector3f{ -1, -1, 1 },
        Vector3f{ 1, -1, 1 },
        Vector3f{ 1, 1, 1 } };
    auto const indexData = vector<unsigned int>{ 0, 1, 2, 3, 0, 4, 5, 1, 0, 3, 7, 4, 2, 6, 7, 3, 1, 5, 6, 2, 7, 6, 5, 4 };

    glGenVertexArrays(1, &vertexBuffer._vao);
    glBindVertexArray(vertexBuffer._vao);
    skyBox->SetVao(vertexBuffer._vao);

    glGenBuffers(1, &vertexBuffer._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector3f), vertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer._veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

auto ResourceManager::LoadMaterials(vector<Material *> const& materials) -> void {
    _materialBuffers.emplace_back();
    auto & materialUbo = _materialBuffers.back();
    glGenBuffers(1, &materialUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, materialUbo);

    glBufferData(GL_UNIFORM_BUFFER, Material::ShaderData::Size() * materials.size(), nullptr, GL_DYNAMIC_DRAW);
    auto * p = static_cast<unsigned char*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
    auto offset = 0;
    for (auto & material : materials) {
        material->SetUbo(materialUbo);
        material->SetUboOffset(offset);
        memcpy(p, &material->GetShaderData(), sizeof(Material::ShaderData));
        p += Material::ShaderData::Size();
        offset += Material::ShaderData::Size();
    }
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto ResourceManager::LoadModels(vector<Model*> const & models) -> void {
    _transformBuffers.emplace_back();
    auto & transformUbo = _transformBuffers.back();
    glGenBuffers(1, &transformUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, transformUbo);

    auto cache = vector<unsigned char>(Model::ShaderData::Size() * models.size());
    auto offset = 0;
    for (auto & model : models) {
        model->SetUbo(transformUbo);
        model->SetUboOffset(offset);
        auto * p = reinterpret_cast<Movable::ShaderData *>(cache.data() + offset);
        *p = model->GetShaderData();
        offset += Model::ShaderData::Size();
    }
    //glBufferData(GL_UNIFORM_BUFFER, cache.size(), cache.data(), GL_DYNAMIC_DRAW);
    glBufferStorage(GL_UNIFORM_BUFFER, cache.size(), cache.data(), 0);
    // may use glBufferStorage() together with glBufferSubData() to update transform data for dynamic objects
    //glBufferStorage(GL_UNIFORM_BUFFER, cache.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);
    //glBufferSubData(GL_UNIFORM_BUFFER, 0, cache.size(), cache.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto ResourceManager::LoadTexture(Texture * texture) -> void {
    _textures.emplace_back();
    auto & tex = _textures.back();
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    texture->SetTexture(tex);

    auto width = texture->GetWidth();
    auto height = texture->GetHeight();
    auto levelCount = static_cast<GLsizei>(floor(log2(std::max(width, height))) + 1);
    glTexStorage2D(GL_TEXTURE_2D, levelCount, GL_RGBA32F, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, texture->GetData().data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    // todo: wrap glGetError(), in release code, calling it only once per frame untill error happpens
    auto error = glGetError();
    assert(GL_NO_ERROR == error);
}

auto ResourceManager::LoadCubeMap(CubeMap * cubeMap) -> void {
    _textures.emplace_back();
    auto & texture = _textures.back();

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    cubeMap->SetTexture(texture);
    auto width = cubeMap->GetWidth();
    auto height = cubeMap->GetHeight();
    auto levelCount = static_cast<GLsizei>(floor(log2(std::max(width, height))) + 1);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, levelCount, GL_RGBA32F, width, height);
    auto const& data = cubeMap->GetData();
    for (auto i = 0u; i < 6; ++i) {
        auto target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
        glTexSubImage2D(target, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data[i].data());
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    if (GL_NO_ERROR != glGetError()) {
        MessageLogger::Log(MessageLogger::Error, "generating cube map failed.");
    }
}

auto ResourceManager::LoadTextureArray(TextureArray * textureArray) -> void {
    _textures.emplace_back();
    auto & texture = _textures.back();
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    textureArray->SetTexture(texture);

    auto width = textureArray->GetWidth();
    auto height = textureArray->GetHeight();
    auto levelCount = static_cast<GLsizei>(floor(log2(std::max(width, height))) + 1);
    auto const& data = textureArray->GetData();
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, levelCount, GL_RGBA32F, width, height, data.size());
    for (auto i = 0u; i < data.size(); ++i) {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_FLOAT, data[i].data());
    }
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // repeat texture(suppose it's seamless)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    auto error = glGetError();
    assert(GL_NO_ERROR == error);
}

auto ResourceManager::LoadAabbs(std::vector<Aabb *> aabbs) -> void {
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();

    glGenVertexArrays(1, &vertexBuffer._vao);
    glBindVertexArray(vertexBuffer._vao);

    glGenBuffers(1, &vertexBuffer._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);

    glGenBuffers(1, &vertexBuffer._veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);

    auto vertexData = vector<Vector3f>{};
    auto indexData = vector<unsigned int>{};
    for (auto aabb : aabbs) {
        aabb->SetRenderData(Aabb::RenderData{
            vertexBuffer._vao,
            indexData.size() * sizeof(unsigned int),
            static_cast<GLint>(vertexData.size()),
        });
        auto const minVertex = aabb->GetMinVertex();
        auto const maxVertex = aabb->GetMaxVertex();
        vertexData.insert(vertexData.end(), {
            { minVertex(0), minVertex(1), minVertex(2) },
            { minVertex(0), maxVertex(1), minVertex(2) },
            { minVertex(0), maxVertex(1), maxVertex(2) },
            { minVertex(0), minVertex(1), maxVertex(2) },
            { maxVertex(0), minVertex(1), minVertex(2) },
            { maxVertex(0), maxVertex(1), minVertex(2) },
            { maxVertex(0), maxVertex(1), maxVertex(2) },
            { maxVertex(0), minVertex(1), maxVertex(2) },
        });
        auto const index = vector<unsigned int>{
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7,
        };
        indexData.insert(indexData.end(), index.begin(), index.end());
    }
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector3f), vertexData.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

auto ResourceManager::LoadTerrain(Terrain * terrain) -> void {
    auto const& shaderData = terrain->GetShaderData();
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();
    glGenVertexArrays(1, &vertexBuffer._vao);
    glBindVertexArray(vertexBuffer._vao);
    terrain->SetVao(vertexBuffer._vao);

    glGenBuffers(1, &vertexBuffer._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);
    auto tileSize = shaderData.tileSize;
    auto tileVertexData = std::array<Vector2f, 4>{ Vector2f{ 0, 0 }, Vector2f{ tileSize, 0 }, Vector2f{ tileSize, tileSize }, Vector2f{ 0, tileSize } };
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector2f), tileVertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vertexBuffer._veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);
    auto indexData = std::array<unsigned int, 6>{ 0, 1, 3, 1, 2, 3 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    // instance data
    _instanceBuffers.emplace_back();
    auto & vio = _instanceBuffers.back();
    glGenBuffers(1, &vio);
    glBindBuffer(GL_ARRAY_BUFFER, vio);
    terrain->SetVio(vio);
    auto sightDistance = shaderData.sightDistance;
    auto tileCountUpperBound = static_cast<int>(ceil(sightDistance / tileSize) * 2);
    tileCountUpperBound *= tileCountUpperBound;
    glBufferData(GL_ARRAY_BUFFER, tileCountUpperBound * sizeof(Vector3f), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // terrain ubo
    _uniformBuffers.emplace_back();
    auto & ubo = _uniformBuffers.back();
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, Terrain::ShaderData::Size(), &shaderData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Terrain), ubo);

    auto error = glGetError();
    assert(error == GL_NO_ERROR);
}

auto ResourceManager::LoadTerrainSpecialTiles(std::vector<Mesh*> terrainSpecialTiles) -> void {
    _vertexBuffers.emplace_back();
    auto & vertexBuffer = _vertexBuffers.back();
    glGenVertexArrays(1, &vertexBuffer._vao);
    glBindVertexArray(vertexBuffer._vao);

    glGenBuffers(1, &vertexBuffer._vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer._vbo);

    glGenBuffers(1, &vertexBuffer._veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer._veo);

    auto vertexData = vector<Vector2f>();
    auto indexData = vector<unsigned int>();
    for (auto mesh : terrainSpecialTiles) {
        mesh->SetRenderData(Mesh::RenderData{
            vertexBuffer._vao,
            indexData.size() * sizeof(unsigned int),
            static_cast<openglInt>(vertexData.size()),
        });
        for (auto const& vertex : mesh->GetVertex()) {
            vertexData.push_back(Vector2f{ vertex.coord(0), vertex.coord(1) });
        }
        indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
    }
    auto tileCoordData = vector<Vector2i>(vertexData.size(), Vector2i{ 0, 0 });
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector2f) + tileCoordData.size() * sizeof(Vector2i), vertexData.data(), GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(Vector2f), vertexData.data());
    glBufferSubData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector2f), tileCoordData.size() * sizeof(Vector2i), tileCoordData.data());

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(1, 2, GL_INT, sizeof(Vector2i), (GLvoid*)(vertexData.size() * sizeof(Vector2f)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    auto error = glGetError();
    assert(error == GL_NO_ERROR);
}

auto ResourceManager::UpdateTerrainTileCoordUbo(openglUint vio, std::vector<Vector3f> const & tileCoord) -> void {
    glBindBuffer(GL_ARRAY_BUFFER, vio);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vector3f) * tileCoord.size(), tileCoord.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
*/
}
