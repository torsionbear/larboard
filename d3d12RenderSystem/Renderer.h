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
    virtual auto Draw(core::Viewpoint const* camera, core::SkyBox const* skyBox, core::Terrain const* terrain, core::Shape const*const* shapes, unsigned int shapeCount, core::Viewpoint const * shadowCastingLightViewpoint) -> void;
    auto DrawTranslucent(core::Shape const*const* shapes, unsigned int shapeCount) -> void;
    virtual auto AllocateDescriptorHeap(
        unsigned int cameraCount,
        unsigned int meshCount,
        unsigned int modelCount,
        unsigned int textureCount,
        unsigned int materialCount,
        unsigned int skyBoxCount,
        unsigned int terrainCount,
        unsigned int nullDescriptorCount) -> void;
    auto DrawSkyBox(core::SkyBox const* skyBox) -> void;
    auto DrawTerrain(core::Terrain const * terrain) -> void;
    auto DrawShadowMap(core::Viewpoint const * viewpoint, core::Shape const*const* shapes, unsigned int shapeCount) -> void;
    auto UseViewpoint(core::Viewpoint const* viewpoint) -> void;
    auto UseLight() -> void;
protected:
    auto DrawShapeWithPso(core::Shape const* shape, ID3D12PipelineState * pso) -> void;
    auto CreateDefaultPso() -> void;
    auto CreateSkyBoxPso() -> void;
    auto CreateTerrainPso() -> void;
    auto CreateTerrainWireframePso() -> void;
    auto CreateTranslucentPso() -> void;
    auto CreateShadowMapPso() -> void;
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
    ID3D12PipelineState * _currentPso;

    DescriptorInfo _depthStencil;
    DescriptorInfo _shadowMapDepthStencil;
    DescriptorInfo _shadowMapDepthStencilSrv;
    core::Vector2i _shadowMapSize = core::Vector2i{2048, 2048};

    bool _wireframeMode = false;
};

}