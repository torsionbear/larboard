#pragma once

#include "ResourceManager.h"

#include "core/Renderer.h"
#include "core/Shape.h"

namespace d3d12RenderSystem {

class Renderer : public core::IRenderer {
public:
    virtual auto Prepare() -> void override;
    virtual auto DrawBegin() -> void override;
    virtual auto DrawEnd() -> void override;
    virtual auto ToggleWireframe() -> void override;
    virtual auto ToggleBackFace() -> void override;
    auto Init(ResourceManager * _resourceManager, unsigned int width, unsigned int height) -> void;
    auto RenderShape(core::Shape const* shape) -> void;

    auto Update(core::Camera const& camera) -> void;
private:
    ResourceManager * _resourceManager;
    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;
    
    // render logic specific data members
    D3D12_CPU_DESCRIPTOR_HANDLE _dsv;
    ConstantBuffer _constantBuffer;
};

}