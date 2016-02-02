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
    struct ShaderData {
        Vector2i heightMapOrigin;
        Vector2i heightMapSize;
        Vector2i diffuseMapOrigin;
        Vector2i diffuseMapSize;
        Float32 tileSize;
        Float32 sightDistance;
    public:
        static auto Size() -> unsigned int;
    };
public:
    Terrain(std::vector<std::string> && diffuseMapFiles, std::string heightMap);
public:
    auto Load(Float32 sightDistance) -> void;
    auto SetTileSize(Float32 tileSize) -> void {
        _shaderData.tileSize = tileSize;
    }
    auto SetHeightMapOrigin(Vector2i heightMapOrigin) -> void {
        _shaderData.heightMapOrigin = heightMapOrigin;
    }
    auto SetHeightMapSize(Vector2i heightMapSize) -> void {
        _shaderData.heightMapSize = heightMapSize;
    }
    auto SetDiffuseMapOrigin(Vector2i diffuseMapOrigin) -> void {
        _shaderData.diffuseMapOrigin = diffuseMapOrigin;
    }
    auto SetDiffuseMapSize(Vector2i diffuseMapSize) -> void {
        _shaderData.diffuseMapSize = diffuseMapSize;
    }
    auto GetShaderProgram() const -> ShaderProgram const* {
        return &_shaderProgram;
    }
    auto GetShaderProgram() -> ShaderProgram * {
        return &_shaderProgram;
    }
    auto GetShaderData() const -> ShaderData const& {
        return _shaderData;
    }
    auto GetVao() const -> openglUint {
        return _vao;
    }
    auto SetVao(openglUint vao) {
        _vao = vao;
    }
    auto GetVio() -> openglUint {
        return _vio;
    }
    auto SetVio(openglUint vio) {
        _vio = vio;
    }
    auto AddSpecialTiles(std::vector<std::unique_ptr<Shape>> && shapes, std::vector<std::unique_ptr<Mesh>> && meshes) -> void;
    auto GetHeight(Vector2f coord) const->Float32;
    auto GetDiffuseMap() const -> TextureArray const* {
        return &_diffuseMap;
    }
    auto GetDiffuseMap() -> TextureArray * {
        return &_diffuseMap;
    }
    auto GetHeightMap() const -> Texture const* {
        return &_heightMap;
    }
    auto GetHeightMap() -> Texture * {
        return &_heightMap;
    }
    auto GetTileCount() const -> unsigned int {
        return _tileCoord.size();
    }
    auto GetSpecialTiles() const -> std::vector<Mesh *>;
    auto GetTileCoordinate(Camera const* camera) -> std::vector<Vector3f> const&;
private:
    auto GetViewFrustumCoverage(Camera const* camera) const -> std::array<Vector2f, 2>;
private:
    ShaderData _shaderData = {
        Vector2i{ -5, -5 },
        Vector2i{ 10, 10 },
        Vector2i{ 0, 0 },
        Vector2i{ 1, 1 },
        10.0f,
        200.0f,
    };

    std::vector<Vector2i> _holeTiles;
    std::vector<std::unique_ptr<Shape>> _specialTileShapes;
    std::vector<std::unique_ptr<Mesh>> _specialTileMeshes;
    std::vector<Vector3f> _tileCoord;

    TextureArray _diffuseMap;
    Texture _heightMap;
    ShaderProgram _shaderProgram;
    openglUint _vao;
    openglUint _vio;
};

}