#pragma once

#include <vector>
#include <memory>

//#include "Mesh.h"
//#include "Skybox.h"
//#include "Bvh.h"
//#include "Material.h"
//#include "Model.h"
//#include "TextureArray.h"
//#include "Terrain.h"

#include <d3d12.h>
//#include <dxgi1_4.h>

#include <wrl.h>

#include "core/Mesh.h"
#include "core/Model.h"
#include "core/Camera.h"
#include "core/Texture.h"
#include "core/AmbientLight.h"
#include "core/PointLight.h"
#include "core/DirectionalLight.h"
#include "core/SpotLight.h"
#include "core/Material.h"
#include "core/Skybox.h"
#include "core/Terrain.h"
#include "SwapChainRenderTargets.h"
#include "FrameResource.h"
#include "FencedCommandQueue.h"
#include "UploadHeap.h"
#include "DescriptorHeap.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct RootSignatureParameterIndex {
    enum {
        SrvAll1,
        SrvPsArray,
        Transform,
        Material,
        Camera,
        Light,
        CbvAll,
        CbvPs2,
        TextureIndex,
        Count,
  };
};

struct SrvRegisterConvention {
    enum {
        SrvAll1,
        SrvPsArray,
        Count,
    };
};

struct CbvRegisterConvention {
    enum {
        Camera = 0u,
        Transform,
        Light,
        Material,
        CbvAll,
        Ps2,
        TextureIndex,
        Count,
    };
};

struct SamplerRegisterConvention {
    enum {
        sampler0,
        sampler1,
    };
};

struct TextureIndex {
    enum {
        slot0,
        slot1,
        slot2,
        slot3,
        slot4,
        slot5,
        slot6,
        count,
    };
};

struct MeshDataInfo {
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
    unsigned int indexCount;
    unsigned int indexOffset;
    int baseVertex;
    D3D12_VERTEX_BUFFER_VIEW instanceVbv;
    unsigned int instanceCount;
    unsigned int instanceOffset;
};
// constant buffer data members are 4*4 bytes packed, constant buffer itself must be D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT(256) bytes aligned.
struct CameraData {
    core::Matrix4x4f viewTransform;
    core::Matrix4x4f projectTransform;
    core::Matrix4x4f viewTransformInverse;
    core::Vector4f viewPosition;
    core::Matrix3x4f _pad;
};

struct TransformData {
    core::Matrix4x4f worldTransform;
    core::Matrix4x4f normalTransform;
    core::Matrix4x4f _pad1;
    core::Matrix4x4f _pad2;
};

struct MaterialData {
    core::Vector3f diffuse;
    int32 hasDiffuseMap;
    core::Vector3f emissive;
    int32 hasEmissiveMap;
    core::Vector3f specular;
    int32 hasSpecularMap;
    core::Float32 shininess;
    int32 hasNormalMap;
    core::Float32 transparency;
    core::Float32 _pad1;
    core::Matrix4x4f _pad2[3];
};

struct LightData {
    enum {
        MaxAmbientLightCount = 3,
        MaxDirectionalLightCount = 6u,
        MaxPointLightCount = 64u,
        MaxSpotLightCount = 64u,
    };
    uint32 ambientLightCount;
    uint32 directionalLightCount;
    uint32 pointLightCount;
    uint32 spotLightCount;
    struct AmbientLight {
        core::Vector4f color;
    } ambientLights[MaxAmbientLightCount];
    struct DirectionalLight {
        core::Vector4f color;
        core::Vector4f direction;
    } directionalLights[MaxDirectionalLightCount];
    struct PointLight {
        core::Vector4f color;
        core::Vector4f position;
        core::Vector4f attenuation;
    } pointLights[MaxPointLightCount];
    struct SpotLight {
        core::Vector4f color;
        core::Vector4f position;
        core::Vector4f direction;
        core::Vector4f attenuation;
        core::Vector4f coneShape; // beamWidth, cutOffAngle, unused, unused
    } spotLights[MaxSpotLightCount];
};

struct AmbientLightData {
    core::Vector4f color;
    core::Vector4f _pad0[3];
    core::Matrix4x4f _pad1[3];
};

struct DirectionalLightData {
    core::Vector4f color;
    int32 isShadowCastingLight;
    core::Vector3f _pad0;
    core::Vector4f _pad1[2];
    core::Matrix4x4f _pad2[3];
};

struct PointLightData {
    core::Vector4f color;
    core::Vector4f attenuation;
    core::Vector4f _pad0[2];
    core::Matrix4x4f _pad1[3];
};

struct SpotLightData {
    core::Vector4f color;
    core::Vector4f attenuation;
    core::Vector4f coneShape; // beamWidth, cutOffAngle, unused, unused
    core::Vector4f _pad0;
    core::Matrix4x4f _pad1[3];
};

struct TerrainData {
    core::Vector2i32 heightMapOrigin;
    core::Vector2i32 heightMapSize;
    core::Vector2i32 diffuseMapOrigin;
    core::Vector2i32 diffuseMapSize;
    Float32 tileSize;
    Float32 sightDistance;
    core::Vector2f _pad1;
    core::Vector4f _pad2;
    core::Matrix4x4f _pad3[3];
};

class ResourceManager {
public:
    auto static CreateDevice(IDXGIFactory1 * factory)->ComPtr<ID3D12Device>;
public:
    ResourceManager(IDXGIFactory1 * factory, unsigned int width, unsigned int height, HWND hwnd);
    ~ResourceManager() {
        _fencedCommandQueue.SyncLatest();
    }
public:
    auto PrepareResource() -> void;
    auto LoadBegin() -> void;
    auto LoadEnd() -> void;
    template<typename T>
    auto LoadMeshes(core::Mesh<T> ** meshes, unsigned int count) -> void;
    auto LoadMovables(core::Movable ** movables, unsigned int count, ID3D12Resource * buffer = nullptr) -> void;
    auto LoadCamera(core::Camera * camera, unsigned int count) -> void;
    auto UpdateViewpoint(core::Viewpoint const * viewpoint) -> void;
    auto LoadLight(
        core::AmbientLight ** ambientLights, unsigned int ambientLightCount,
        core::DirectionalLight ** directionalLights, unsigned int directionalLightCount,
        core::PointLight ** pointLights, unsigned int pointLightCount,
        core::SpotLight ** spotLights, unsigned int spotLightCount) -> void;
    auto LoadAmbientLight(core::AmbientLight * ambientLights) -> void;
    auto LoadDirectionalLight(core::DirectionalLight ** directionalLights, unsigned int directionalLightCount) -> void;
    auto LoadPointLight(core::PointLight ** pointLights, unsigned int pointLightCount) -> void;
    auto LoadSpotLight(core::SpotLight ** spotLights, unsigned int spotLightCount) -> void;
    auto LoadShadowCastingLight(core::DirectionalLight ** directionalLights, unsigned int directionalLightCount) -> void;
    auto LoadMaterials(core::Material ** materials, unsigned int count) -> void;
    auto LoadTexture(core::Texture * texture) -> void;
    auto LoadDdsTexture(core::Texture ** texture, unsigned int count) -> void;
    auto LoadDdsTexture(std::string const& filename) -> unsigned int;
    auto LoadSkyBox(core::SkyBox * skybox) -> void;
    auto LoadTerrain(core::Terrain * terrain) -> void;
    auto CreateDepthStencil(unsigned int width, unsigned int height, DescriptorInfo * srv) -> DescriptorInfo;
    auto CreatePso(D3D12_GRAPHICS_PIPELINE_STATE_DESC const* psoDesc) -> ComPtr<ID3D12PipelineState>;
    auto CompileShader(std::string const& filename, std::string const& target)->ComPtr<ID3DBlob>;
    auto CreateRenderTarget(DXGI_FORMAT format, unsigned int width, unsigned int height, uint8 size, DescriptorInfo * srv) -> DescriptorInfo;
    auto CreateTexture2d(DXGI_FORMAT format, uint64 width, uint32 height, void const* data, uint32 size, uint8 stride)->DescriptorInfo;
    auto UploadConstantBufferData(unsigned int size, void const* data, ID3D12Resource * dest = nullptr) -> DescriptorInfo;
    auto UploadVertexData(unsigned int size, unsigned int stride, void const* data, ID3D12Resource ** dest = nullptr) -> D3D12_VERTEX_BUFFER_VIEW;
    auto UploadIndexData(unsigned int size, void const* data) -> D3D12_INDEX_BUFFER_VIEW;
    auto UpdateTerrain(core::Terrain * terrain, core::Camera * camera) -> void;
    auto CreateBundle(ID3D12PipelineState * pso, ID3D12RootSignature * rootSignature, ID3D12DescriptorHeap *const* descriptorHeaps, unsigned int descriptorHeapCount) -> ComPtr<ID3D12GraphicsCommandList>;
    auto GetMeshDataInfo(unsigned int index) -> MeshDataInfo const& {
        return _meshDataInfos[index];
    }
    auto GetRootSignature() -> ID3D12RootSignature * {
        return _rootSignature.Get();
    }
    auto GetCommandList() -> ID3D12GraphicsCommandList * {
        return _commandList.Get();
    }
    auto GetFrameResource() -> FrameResource& {
        return _frameResourceContainer.GetCurrent();
    }
    auto GetSwapChainRenderTargets() -> SwapChainRenderTargets & {
        return _swapChainRenderTargets;
    }
    auto GetFencedCommandQueue() -> FencedCommandQueue * {
        return &_fencedCommandQueue;
    }
    auto GetCbvSrvDescriptorHeap() const -> DescriptorHeap const& {
        return _cbvSrvHeap;
    }
    auto GetDsvDescriptorHeap() const -> DescriptorHeap const& {
        return _dsvHeap;
    }
    auto AllocCbvSrvDescriptorHeap(unsigned int size) -> void {
        _cbvSrvHeap.Init(_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    }
    auto AllocDsvDescriptorHeap(unsigned int size) -> void {
        _dsvHeap.Init(_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, size, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    }
    auto AllocRtvDescriptorHeap(unsigned int size) -> void {
        _rtvHeap.Init(_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, size, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    }
    auto GetCameraDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _cameraDescriptorInfos[index];
    }
    auto GetTransformDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _transformDescriptorInfos[index];
    }
    auto GetTextureDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _textureDescriptorInfos[index];
    }
    auto GetLightDescriptorInfo() -> DescriptorInfo const& {
        return _lightDescriptorInfo;
    }
    auto GetMaterialDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _materialDescriptorInfos[index];
    }
    auto GetNullDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _nullDescriptorInfo[index];
    }
    auto GetSkyBoxMeshInfo() const -> MeshDataInfo const& {
        return _skyBoxMeshInfo;
    }
    auto GetTerrainMeshInfo() const -> MeshDataInfo const& {
        return _terrainMeshInfo;
    }
    auto GetTerrainCbv() const -> DescriptorInfo const& {
        return _terrainCbv;
    }
    auto GetAmbientLightDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _ambientLightDescriptorInfos[index];
    }
    auto GetDirectionalLightDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _directionalLightDescriptorInfos[index];
    }
    auto GetPointLightDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _pointLightDescriptorInfos[index];
    }
    auto GetSpotLightDescriptorInfo(unsigned int index) -> DescriptorInfo const& {
        return _spotLightDescriptorInfos[index];
    }
private:
    auto CreateRootSignature()->ComPtr<ID3D12RootSignature>;
    auto CreateCommandList(ID3D12PipelineState * pso, ID3D12CommandAllocator * allocator)->ComPtr<ID3D12GraphicsCommandList>;
    auto CreateCommittedResource(
        D3D12_RESOURCE_DESC const* desc,
        D3D12_RESOURCE_STATES resourceState,
        D3D12_HEAP_TYPE heapType,
        D3D12_CLEAR_VALUE * clearValue = nullptr) -> ID3D12Resource *;
private:
    ComPtr<ID3D12Device> _device;
    UploadHeap _uploadHeap;

    DescriptorHeap _dsvHeap;
    DescriptorHeap _cbvSrvHeap;
    DescriptorHeap _rtvHeap;
    std::vector<MeshDataInfo> _meshDataInfos;
    std::vector<DescriptorInfo> _cameraDescriptorInfos;
    std::vector<DescriptorInfo> _transformDescriptorInfos;
    std::vector<DescriptorInfo> _textureDescriptorInfos;
    DescriptorInfo _lightDescriptorInfo;
    std::vector<DescriptorInfo> _ambientLightDescriptorInfos;
    std::vector<DescriptorInfo> _directionalLightDescriptorInfos;
    std::vector<DescriptorInfo> _pointLightDescriptorInfos;
    std::vector<DescriptorInfo> _spotLightDescriptorInfos;
    std::vector<DescriptorInfo> _nullDescriptorInfo;
    std::vector<DescriptorInfo> _materialDescriptorInfos;
    MeshDataInfo _skyBoxMeshInfo;
    MeshDataInfo _terrainMeshInfo;
    DescriptorInfo _terrainCbv;
    ID3D12Resource * _terrainInstanceDataResource = nullptr;

    std::vector<ComPtr<ID3D12Resource>> _uploadBuffers;
    std::vector<ComPtr<ID3D12Resource>> _defaultBuffers;

    FrameResourceContainer<FrameResource, 2> _frameResourceContainer;
    SwapChainRenderTargets _swapChainRenderTargets;
    FencedCommandQueue _fencedCommandQueue;

    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12RootSignature> _rootSignature;

    std::vector<ComPtr<ID3D12CommandAllocator>> _commandAllocators;
};

template<typename T>
auto ResourceManager::LoadMeshes(core::Mesh<T> ** meshes, unsigned int count) -> void {
    if (meshes == nullptr || count == 0) {
        return;
    }
    auto vertexBufferSize = 0u;
    auto indexBufferSize = 0u;
    for (auto i = 0u; i < count; ++i) {
        vertexBufferSize += meshes[i]->GetVertex().size() * sizeof(T);
        indexBufferSize += meshes[i]->GetIndex().size() * sizeof(unsigned int);
    }
    auto vertexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    auto const vbv = D3D12_VERTEX_BUFFER_VIEW{ vertexBuffer->GetGPUVirtualAddress(), vertexBufferSize, sizeof(T) };

    auto indexBuffer = CreateCommittedResource(&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
    auto const ibv = D3D12_INDEX_BUFFER_VIEW{ indexBuffer->GetGPUVirtualAddress(), indexBufferSize, DXGI_FORMAT_R32_UINT };

    auto vertexData = vector<T>();
    auto indexData = vector<unsigned int>();
    for (auto i = 0u; i < count; ++i) {
        auto mesh = meshes[i];
        mesh->SetRenderDataId(_meshDataInfos.size());
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

}