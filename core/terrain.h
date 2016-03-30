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
        _tileSize = tileSize;
    }
    auto GetTileSize() const -> Float32 {
        return _tileSize;
    }
    auto SetHeightMapOrigin(Vector2i heightMapOrigin) -> void {
        _heightMapOrigin = heightMapOrigin;
    }
    auto GetHeightMapOrigin() const -> Vector2i {
        return _heightMapOrigin;
    }
    auto SetHeightMapSize(Vector2i heightMapSize) -> void {
        _heightMapSize = heightMapSize;
    }
    auto GetHeightMapSize() const -> Vector2i {
        return _heightMapSize;
    }
    auto SetDiffuseMapOrigin(Vector2i diffuseMapOrigin) -> void {
        _diffuseMapOrigin = diffuseMapOrigin;
    }
    auto GetDiffuseMapOrigin() const -> Vector2i {
        return _diffuseMapOrigin;
    }
    auto SetDiffuseMapSize(Vector2i diffuseMapSize) -> void {
        _diffuseMapSize = diffuseMapSize;
    }
    auto GetDiffuseMapSize() const -> Vector2i {
        return _diffuseMapSize;
    }
    auto GetShaderProgram() const -> ShaderProgram const* {
        return &_shaderProgram;
    }
    auto GetShaderProgram() -> ShaderProgram * {
        return &_shaderProgram;
    }
    auto GetShaderData() const -> ShaderData const& {
        return ShaderData{
            _heightMapOrigin,
            _heightMapSize,
            _diffuseMapOrigin,
            _diffuseMapSize,
            _tileSize,
            _sightDistance,
        };
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
    auto AddSpecialTiles(std::vector<std::unique_ptr<Shape>> && shapes, std::vector<std::unique_ptr<Mesh<Vertex>>> && meshes) -> void;
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
    auto GetDiffuseMapFilename() const -> std::string {
        return _diffuseMapFilename;
    }
    auto GetHeightMapFilename() const -> std::string {
        return _heightMapFilename;
    }
    auto SetSightDistance(Float32 sightDistance) -> void {
        _sightDistance = sightDistance;
    }
    auto GetSightDistance() const -> Float32 {
        return _sightDistance;
    }
    auto GetSpecialTiles() const->std::vector<Mesh<Vertex> *>;
    auto GetTileCoordinate(Camera const* camera)->std::vector<Vector3f> const&;
    auto GetTileCoordinateWithSpecialTiles(Camera const* camera) -> std::vector<Vector3f> const&;
private:
    auto CalculateTileCoordinate(Camera const* camera, std::vector<core::Vector3f> & tileCoords) -> void;
    auto GetViewFrustumCoverage(Camera const* camera) const->std::array<Vector2f, 2>;
private:
    Vector2i _heightMapOrigin = { -5, -5 };
    Vector2i _heightMapSize = { 10, 10 };
    Vector2i _diffuseMapOrigin = { 0, 0 };
    Vector2i _diffuseMapSize = { 1, 1, };
    Float32 _tileSize = 10.0f;
    Float32 _sightDistance = 1000.0f;

    std::vector<Vector2i> _holeTiles;
    std::vector<std::unique_ptr<Shape>> _specialTileShapes;
    std::vector<std::unique_ptr<Mesh<Vertex>>> _specialTileMeshes;
    std::vector<Vector3f> _tileCoord;

    TextureArray _diffuseMap;
    std::string _diffuseMapFilename = "media/terrain/diffuse.dds";;
    Texture _heightMap;
    std::string _heightMapFilename = "media/terrain/heightMap.dds";
    ShaderProgram _shaderProgram;
    openglUint _vao;
    openglUint _vio;
};

}