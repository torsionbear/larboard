#pragma once

#include "Renderer.h"

#include "core/Renderer.h"
#include "core/Shape.h"

namespace d3d12RenderSystem {

class SsaoRenderer : public Renderer {
private:
    struct SsaoData {
        //core::Vector2i randomVectorTextureSize;
        core::Vector2i occlusionTextureSize;
        int sampleCount;
        int _pad;
        std::array<core::Vector4f, 95> samples;
    };
public:
    SsaoRenderer(ResourceManager * resourceManager, unsigned int width, unsigned int height)
        : Renderer(resourceManager, width, height) {
    }
public:
    virtual auto Prepare() -> void override;
    virtual auto Draw(
        core::Viewpoint const* camera,
        core::SkyBox const* skyBox,
        core::Terrain const* terrain,
        core::Shape const*const* shapes,
        unsigned int shapeCount,
        core::Viewpoint const * shadowCastingLightViewpoint,
        core::AmbientLight * ambientLight,
        core::DirectionalLight ** directionalLights, unsigned int directionalLightCount,
        core::PointLight ** pointLights, unsigned int pointLightCount,
        core::SpotLight ** spotLights, unsigned int spotLightCount) -> void override;
    virtual auto AllocateDescriptorHeap(
        unsigned int cameraCount,
        unsigned int meshCount,
        unsigned int modelCount,
        unsigned int textureCount,
        unsigned int materialCount,
        unsigned int skyBoxCount,
        unsigned int terrainCount,
        unsigned int directionalLightCount,
        unsigned int pointLightCount,
        unsigned int spotLightCount,
        unsigned int nullDescriptorCount) -> void override;
private:
    auto GenerateRandomTexture(unsigned int textureSize) -> void;
    auto GenerateSamples(unsigned int sampleCount, core::Vector2f sampleDistanceRange) -> void;
    auto PopulateSsaoData() -> void;
    auto CreateSsaoDefaultPso() -> void;
    auto CreateSsaoPassPso() -> void;
    auto CreateLightingPassPso() -> void;
    auto CreateAmbientLightPso() -> void;
    auto CreateDirectionalLightPso() -> void;
    auto CreatePointLightPso() -> void;
    auto CreateSpotLightPso() -> void;
    auto LoadScreenQuad() -> void;
    auto LoadPointLightVolume() -> void;
    auto DrawAmbientLight(ID3D12GraphicsCommandList * commandList, core::AmbientLight * ambientLight, DescriptorInfo ambientOcclusionSrv) -> void;
    auto DrawDirectionalLights(ID3D12GraphicsCommandList * commandList, core::DirectionalLight ** directionalLights, unsigned int directionalLightCount) -> void;
    auto DrawPointLights(ID3D12GraphicsCommandList * commandList, core::PointLight ** pointLights, unsigned int pointLightCount) -> void;
    auto DrawSpotLights(ID3D12GraphicsCommandList * commandList, core::SpotLight ** spotLights, unsigned int spotLightCount) -> void;
private:
    DescriptorInfo _gBufferDiffuse;
    DescriptorInfo _gBufferDiffuseSrv;
    DescriptorInfo _gBufferNormal;
    DescriptorInfo _gBufferNormalSrv;
    DescriptorInfo _gBufferSpecular;
    DescriptorInfo _gBufferSpecularSrv;
    DescriptorInfo _gBufferDepthStencil;
    DescriptorInfo _gBufferDepthStencilSrv;
    DescriptorInfo _randomVectorTexture;
    DescriptorInfo _ambientOcclusion;
    DescriptorInfo _ambientOcclusionSrv;

    unsigned int const _randomTextureSize = 4u;
    unsigned int const _sampleCount = 64u;
    SsaoData _ssaoData;
    DescriptorInfo _ssaoDataCbv;
    ComPtr<ID3D12PipelineState> _ssaoPassPso;
    ComPtr<ID3D12PipelineState> _lightingPassPso;
    ComPtr<ID3D12PipelineState> _ambientLightPso;
    ComPtr<ID3D12PipelineState> _directionalLightPso;
    ComPtr<ID3D12PipelineState> _pointLightPso;
    ComPtr<ID3D12PipelineState> _spotLightPso;

    D3D12_VERTEX_BUFFER_VIEW _screenQuadVbv;
    D3D12_INDEX_BUFFER_VIEW _screenQuadIbv;
};

}