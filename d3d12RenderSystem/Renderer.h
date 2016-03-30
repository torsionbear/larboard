#pragma once

#include "ResourceManager.h"

#include "core/Renderer.h"
#include "core/Shape.h"

namespace d3d12RenderSystem {

class Renderer : public core::IRenderer {
public:
    Renderer(ResourceManager * resourceManager, unsigned int width, unsigned int height);
public:
    virtual auto Prepare() -> void override;
    virtual auto DrawBegin() -> void override;
    virtual auto DrawEnd() -> void override;
    virtual auto ToggleWireframe() -> void override;
    virtual auto ToggleBackFace() -> void override;
    virtual auto Draw(
        core::Viewpoint const* camera,
        core::SkyBox const* skyBox,
        core::Terrain const* terrain,
        core::Shape const*const* shapes, unsigned int shapeCount,
        core::Viewpoint const * shadowCastingLightViewpoint,
        core::AmbientLight * ambientLight,
        core::DirectionalLight ** directionalLights, unsigned int directionalLightCount,
        core::SpotLight ** spotLights, unsigned int spotLightCount) -> void;
    virtual auto AllocateDescriptorHeap(
        unsigned int cameraCount,
        unsigned int meshCount,
        unsigned int movableCount,
        unsigned int textureCount,
        unsigned int materialCount,
        unsigned int skyBoxCount,
        unsigned int terrainCount,
        unsigned int directionalLightCount,
        unsigned int spotLightCount,
        unsigned int nullDescriptorCount) -> void;
    auto DrawTranslucent(core::Shape const*const* shapes, unsigned int shapeCount) -> void;
    auto DrawSkyBox(ID3D12GraphicsCommandList * commandList) -> void;
    auto DrawTerrain(ID3D12GraphicsCommandList * commandList, core::Terrain const * terrain) -> void;
    auto DrawShadowMap() -> void;
    auto UseViewpoint(ID3D12GraphicsCommandList * commandList, core::Viewpoint const* viewpoint) -> void;
    auto UseLight(ID3D12GraphicsCommandList * commandList) -> void;
    auto CreateShadowMapBundle(core::Viewpoint const * viewpoint, core::Shape const*const* shapes, unsigned int shapeCount) -> void;
    auto CreateSkyBoxBundle(core::SkyBox const* skyBox) -> void;
    auto CreateTerrainBundle(core::Terrain const * terrain) -> void;
    auto CreateTranslucentBundle(core::Shape const*const* shapes, unsigned int shapeCount) -> void;
    auto CreateShapeBundle(core::Shape const*const* shapes, unsigned int shapeCount) -> void;
protected:
    auto DrawShapes(ID3D12GraphicsCommandList * commandList) -> void;
    auto DrawShapeWithPso(ID3D12GraphicsCommandList * commandList, core::Shape const* shape, ID3D12PipelineState * pso) -> void;
    auto CreateDefaultPso() -> void;
    auto CreateSkyBoxPso() -> void;
    auto CreateTerrainPso() -> void;
    auto CreateTerrainWireframePso() -> void;
    auto CreateTranslucentPso() -> void;
    auto CreateShadowMapPso() -> void;
    auto CreateTerrainBundle(core::Terrain const * terrain, ID3D12PipelineState * pso) -> ComPtr<ID3D12GraphicsCommandList>;
protected:
    ResourceManager * _resourceManager;
    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12PipelineState> _skyBoxPso;
    ComPtr<ID3D12PipelineState> _terrainPso;
    ComPtr<ID3D12PipelineState> _terrainWireframePso;
    ComPtr<ID3D12PipelineState> _translucentPso;
    ComPtr<ID3D12PipelineState> _shadowMapPso;

    DescriptorInfo _depthStencil;
    DescriptorInfo _shadowMapDepthStencil;
    DescriptorInfo _shadowMapDepthStencilSrv;
    core::Vector2i _shadowMapSize = core::Vector2i{2048, 2048};
    ComPtr<ID3D12GraphicsCommandList> _shadowMapBundle;
    ComPtr<ID3D12GraphicsCommandList> _skyBoxBundle;
    ComPtr<ID3D12GraphicsCommandList> _terrainBundle;
    ComPtr<ID3D12GraphicsCommandList> _terrainWireframeBundle;
    ComPtr<ID3D12GraphicsCommandList> _translucentBundle;
    ComPtr<ID3D12GraphicsCommandList> _shapeBundle;

    bool _wireframeMode = false;
};

}