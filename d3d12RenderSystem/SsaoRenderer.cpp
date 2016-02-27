#include "SsaoRenderer.h"

d3d12RenderSystem::SsaoRenderer::SsaoRenderer(ResourceManager * resourceManager, unsigned int width, unsigned int height) {
    _resourceManager = resourceManager;
    _viewport.Width = static_cast<float>(width);
    _viewport.Height = static_cast<float>(height);
    _viewport.MaxDepth = 1.0f;
    _scissorRect.right = static_cast<LONG>(width);
    _scissorRect.bottom = static_cast<LONG>(height);
}

auto d3d12RenderSystem::SsaoRenderer::Prepare() -> void {
}

auto d3d12RenderSystem::SsaoRenderer::DrawBegin() -> void {
}

auto d3d12RenderSystem::SsaoRenderer::DrawEnd() -> void {
}

auto d3d12RenderSystem::SsaoRenderer::ToggleWireframe() -> void {
}

auto d3d12RenderSystem::SsaoRenderer::ToggleBackFace() -> void {
}
