#pragma once

#include "ResourceManager.h"

#include "core/Shape.h"

namespace d3d12RenderSystem {

class Renderer {
public:
    auto Init(ResourceManager * _resourceManager, unsigned int width, unsigned int height) -> void;
    auto Prepare() -> void;
    auto RenderBegin() -> void;
    auto RenderEnd() -> void;
    auto RenderShape(core::Shape const* shape) -> void;
private:
    ResourceManager * _resourceManager;
    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

};

}