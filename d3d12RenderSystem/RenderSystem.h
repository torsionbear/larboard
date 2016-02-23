#pragma once

#include <memory>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>

#include <wrl.h>

#include "RenderWindow.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "FencedCommandQueue.h"
#include "SwapChainRenderTargets.h"

#include "core/Shape.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct Vertex {
    float position[3];
    float color[4];
};

class RenderSystem {
public:
    ~RenderSystem();
public:
    auto GetResourceManager() -> ResourceManager * {
        return _resourceManager.get();
    }
    auto GetRenderer() -> Renderer * {
        return _renderer.get();
    }
    auto GetRenderWindow() -> RenderWindow & {
        return _renderWindow;
    }
    auto Init(unsigned int width, unsigned int height) -> void;
private:
    auto EnableDebugLayer() -> void;
    auto static CreateFactory() -> ComPtr<IDXGIFactory1>;
    auto static CreateDevice(IDXGIFactory1 * factory) -> ComPtr<ID3D12Device>;

private:
    std::unique_ptr<ResourceManager> _resourceManager;
    std::unique_ptr<Renderer> _renderer;
    RenderWindow _renderWindow;
#if defined(_DEBUG)
    ComPtr<IDXGIDebug1> _dxgiDebug1;
#endif
};

}