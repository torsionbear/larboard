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
    auto RenderShape(core::Shape const* shape) -> void;
    auto RenderSkyBox(core::SkyBox const* skyBox) -> void;
    auto UseCamera(core::Camera const* camera) -> void;
    auto UseLight() -> void;
private:
    auto CreateDefaultPso() -> void;
    auto CreateSkyBoxPso() -> void;
private:
    ResourceManager * _resourceManager;
    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

    ComPtr<ID3D12PipelineState> _defaultPso;
    ComPtr<ID3D12PipelineState> _skyBoxPso;
    ID3D12PipelineState * _currentPso;
};

}