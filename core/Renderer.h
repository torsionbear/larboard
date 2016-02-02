#pragma once

#include "Shape.h"
#include "SkyBox.h"
#include "ShaderProgram.h"
#include "TextureArray.h"
#include "Terrain.h"
#include "ResourceManager.h"

namespace core {

class Renderer {
public:
    virtual ~Renderer() {
    }
public:
    virtual auto Prepare() -> void;
    virtual auto DrawBegin() -> void;
    virtual auto DrawEnd() -> void;
    auto ToggleWireframe() -> void;
    auto ToggleBackFace() -> void;
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