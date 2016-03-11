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
    virtual auto Draw(core::Camera const* camera, core::SkyBox const* skyBox, core::Terrain const* terrain, core::Shape const*const* shapes, unsigned int shapeCount) -> void;
    virtual auto AllocateDescriptorHeap(
        unsigned int cameraCount,
        unsigned int meshCount,
        unsigned int modelCount,
        unsigned int textureCount,
        unsigned int materialCount,
        unsigned int skyBoxCount,
        unsigned int terrainCount,
        unsigned int nullDescriptorCount) -> void;
    auto RenderShape(core::Shape const* shape) -> void;
    auto RenderSkyBox(core::SkyBox const* skyBox) -> void;
    auto DrawTerrain(core::Terrain const * terrain) -> void;
    auto UseCamera(core::Camera const* camera) -> void;
    auto UseLight() -> void;
protected:
    auto CreateDefaultPso() -> void;
    auto CreateSkyBoxPso() -> void;
    auto CreateTerrainPso() -> void;
protected:
    ResourceManager * _resourceManager;
    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12PipelineState> _defaultPsoWireframe;
    ComPtr<ID3D12PipelineState> _skyBoxPso;
    ComPtr<ID3D12PipelineState> _skyBoxPsoWireframe;
    ComPtr<ID3D12PipelineState> _terrainPso;
    ComPtr<ID3D12PipelineState> _terrainPsoWireframe;
    ID3D12PipelineState * _currentPso;

    DescriptorInfo _depthStencil;
};

}