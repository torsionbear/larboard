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
#include "SwapChainRenderTargets.h"
#include "FrameResource.h"
#include "FencedCommandQueue.h"
#include "UploadHeap.h"
#include "DescriptorHeap.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct RootSignatureParameterIndex {
    enum {
        DiffuseMap = 0u,
        NormalMap = 1u,
        SpecularMap = 2u,
        EmissiveMap = 3u,
        Transform = 4u,
        Material = 5u,
        Camera = 6u,
        Light = 7u,
        RootSignatureParameterIndexCount = Light + 1u,
  };
  static auto GetTextureRootSignatureParameterIndex(core::TextureUsage::TextureType type) -> unsigned int {
      switch (type) {
      case core::TextureUsage::DiffuseMap:
          return DiffuseMap;
      case core::TextureUsage::NormalMap:
          return NormalMap;
      case core::TextureUsage::SpecularMap:
          return SpecularMap;
      case core::TextureUsage::EmissiveMap:
          return EmissiveMap;
      }
      assert(false);
      return 0u;
  }
};

struct RegisterConvention {
    enum {
        Camera = 0u,
        Transform = 1u,
        Light = 2u,
        Material = 3u,
    };
    enum {
        DiffuseMap = 0u,
        NormalMap = 1u,
        SpecularMap = 2u,
        EmissiveMap = 3u,
    };
    enum {
        StaticSampler = 0u,
    };
};

struct MeshDataInfo {
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
    unsigned int indexCount;
    unsigned int indexOffset;
    int baseVertex;
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
    core::Vector2f _pad1;
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

struct PersistentMappedBuffer{
    uint8 * _mappedDataPtr;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
};

class ResourceManager {
public:
    ResourceManager() {
    }
public:
    auto Init(ID3D12Device * device, IDXGIFactory1 * factory, FencedCommandQueue * fencedCommandQueue, unsigned int width, unsigned int height, HWND hwnd) -> void;
    auto PrepareResource() -> void;
    auto LoadBegin(unsigned int depthStencilCount, unsigned int cameraCount, unsigned int meshCount, unsigned int modelCount, unsigned int textureCount, unsigned int materialCount) -> void;
    auto LoadEnd() -> void;
    auto LoadMeshes(core::Mesh ** meshes, unsigned int count, unsigned int stride) -> void;
    auto LoadModels(core::Model ** models, unsigned int count) -> void;
    auto LoadCamera(core::Camera * camera, unsigned int count) -> void;
    auto LoadLight(
        core::AmbientLight ** ambientLights, unsigned int ambientLightCount,
        core::DirectionalLight ** directionalLights, unsigned int directionalLightCount,
        core::PointLight ** pointLights, unsigned int pointLightCount,
        core::SpotLight ** spotLights, unsigned int spotLightCount) -> void;
    auto LoadMaterials(core::Material ** materials, unsigned int count) -> void;
    auto LoadTexture(core::Texture * texture) -> void;
    auto LoadDdsTexture(core::Texture * texture) -> void;
    auto UpdateCamera(core::Camera const& camera) -> void;
    auto CreateDepthStencil(unsigned int width, unsigned int height) -> void;

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
        return _fencedCommandQueue;
    }
    auto GetCbvSrvDescriptorHeap() const -> DescriptorHeap const& {
        return _cbvSrvHeap;
    }
    auto GetDsvDescriptorHeap() const -> DescriptorHeap const& {
        return _dsvHeap;
    }
    auto AllocCbvSrvDescriptorHeap(unsigned int size) -> void {
        _cbvSrvHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    }
    auto AllocDsvDescriptorHeap(unsigned int size) -> void {
        _dsvHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, size, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    }
    auto GetCameraBufferInfo(unsigned int index) -> BufferInfo const& {
        return _cameraBufferInfos[index];
    }
    auto GetTransformBufferInfo(unsigned int index) -> BufferInfo const& {
        return _transformBufferInfos[index];
    }
    auto GetDepthStencilBufferInfo(unsigned int index) -> BufferInfo const& {
        return _depthStencilBufferInfos[index];
    }
    auto GetTextureBufferInfo(unsigned int index) -> BufferInfo const& {
        return _textureBufferInfos[index];
    }
    auto GetLightBufferInfo() -> BufferInfo const& {
        return _lightBufferInfo;
    }
    auto GetMaterialBufferInfo(unsigned int index) -> BufferInfo const& {
        return _materialBufferInfos[index];
    }
    auto GetNullBufferInfo(unsigned int index) -> BufferInfo const& {
        return _nullBufferInfo[index];
    }
private:
    auto CreatePso(ID3D12RootSignature * rootSignature)->ComPtr<ID3D12PipelineState>;
    auto CreateRootSignature()->ComPtr<ID3D12RootSignature>;
    auto CreateCommandList(ID3D12PipelineState * pso, ID3D12CommandAllocator * allocator)->ComPtr<ID3D12GraphicsCommandList>;
private:
    ID3D12Device * _device;
    UploadHeap _uploadHeap;

    DescriptorHeap _dsvHeap;
    DescriptorHeap _cbvSrvHeap;
    std::vector<MeshDataInfo> _meshDataInfos;
    std::vector<BufferInfo> _cameraBufferInfos;
    std::vector<BufferInfo> _transformBufferInfos;
    std::vector<BufferInfo> _depthStencilBufferInfos;
    std::vector<BufferInfo> _textureBufferInfos;
    BufferInfo _lightBufferInfo;
    std::vector<BufferInfo> _nullBufferInfo;
    std::vector<BufferInfo> _materialBufferInfos;

    std::vector<ComPtr<ID3D12Resource>> _uploadBuffers;
    std::vector<ComPtr<ID3D12Resource>> _defaultBuffers;

    FrameResourceContainer<FrameResource, 2> _frameResourceContainer;
    SwapChainRenderTargets _swapChainRenderTargets;
    FencedCommandQueue * _fencedCommandQueue;

    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12RootSignature> _rootSignature;
};

}