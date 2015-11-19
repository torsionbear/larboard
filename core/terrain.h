#pragma once

#include <array>

#include "Texture.h"
#include "TextureArray.h"
#include "Camera.h"
#include "ShaderProgram.h"
#include "Shape.h"
#include "Mesh.h"

namespace core {

class Terrain {
public:
    Terrain(std::vector<std::string> && diffuseMapFiles, std::string heightMap);
public:
    auto PrepareForDraw(Float32 sightDistance) -> void;
    auto Draw(Camera const* camera)  -> void;
    auto SetTileSize(Float32 tileSize) -> void {
        _tileSize = tileSize;
    }
    auto SetHeightMapOrigin(Vector2i heightMapOrigin) -> void {
        _heightMapOrigin = heightMapOrigin;
    }
    auto SetHeightMapSize(Vector2i heightMapSize) -> void {
        _heightMapSize = heightMapSize;
    }
    auto SetDiffuseMapOrigin(Vector2i diffuseMapOrigin) -> void {
        _diffuseMapOrigin = diffuseMapOrigin;
    }
    auto SetDiffuseMapSize(Vector2i diffuseMapSize) -> void {
        _diffuseMapSize = diffuseMapSize;
    }
    auto AddSpecialTiles(std::vector<std::unique_ptr<Shape>> && shapes, std::vector<std::unique_ptr<Mesh>> && meshes) -> void;
private:
    auto GetViewFrustumCoverage(Camera const* camera) -> std::array<Vector2f, 2>;
    auto LoadSpecialTiles() -> void;
    auto DrawSpecialTiles() -> void;
private:
    Float32 _tileSize = 10.0f;
    Vector2i _heightMapOrigin = Vector2i{ -5, -5 };
    Vector2i _heightMapSize = Vector2i{ 10, 10 };  // width and height in tile count
    Vector2i _diffuseMapOrigin = Vector2i{ 0, 0 };
    Vector2i _diffuseMapSize = Vector2i{ 1, 1 };
    Float32 _sightDistance = 200.0f;

    std::vector<Vector2i> _holeTiles;
    std::vector<std::unique_ptr<Shape>> _specialTileShapes;
    std::vector<std::unique_ptr<Mesh>> _specialTileMeshes;
    openglUint _specialTilesVao;
    openglUint _specialTilesVbo;
    openglUint _specialTilesVeo;

    TextureArray _diffuseMap;
    Texture _heightMap;
    ShaderProgram _shaderProgram;
    openglUint _vao;
    openglUint _vbo;
    openglUint _veo;
    openglUint _vio;
};

}