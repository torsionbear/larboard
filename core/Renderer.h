#pragma once

#include "Scene.h"
#include "Shape.h"
#include "SkyBox.h"
#include "ShaderProgram.h"
#include "TextureArray.h"
#include "Terrain.h"

namespace core {
class IRenderer {
public:
    virtual ~IRenderer() {
    }
public:
    virtual auto Prepare() -> void = 0;
    virtual auto DrawBegin() -> void = 0;
    virtual auto DrawEnd() -> void = 0;
    virtual auto ToggleWireframe() -> void = 0;
    virtual auto ToggleBackFace() -> void = 0;
};
class Renderer : public IRenderer {
public:
    static auto DrawScene(Renderer * renderer, Scene const* scene) -> void;
public:
    virtual auto Prepare() -> void override;
    virtual auto DrawBegin() -> void override;
    virtual auto DrawEnd() -> void override;
    virtual auto ToggleWireframe() -> void override;
    virtual auto ToggleBackFace() -> void override;
    auto Render(Shape const* shape) -> void;
    auto RenderAabb(Aabb const* aabb) -> void;
    auto RenderSkyBox(SkyBox const * skyBox) -> void;
    auto DrawTerrain(Terrain const* terrain) -> void;
    auto UseTextureArray(TextureArray const* textureArray, TextureUsage::TextureType type) -> void;
    auto UseTexture(Texture const* texture, TextureUsage::TextureType type) -> void;
private:
    auto UseCubeMap(CubeMap const* cubeMap) -> void;
private:
    ShaderProgram const* _currentShaderProgram = nullptr;
    std::vector<std::unique_ptr<ShaderProgram>> _shaderPrograms;
    bool _wireframeMode = false;
    bool _renderBackFace = false;
};

}