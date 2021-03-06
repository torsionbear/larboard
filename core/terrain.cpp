#include "terrain.h"

#include <array>

#include "ResourceManager.h"
#include "GlRuntimeHelper.h"

using std::string;
using std::array;
using std::unique_ptr;

namespace core {

auto Terrain::ShaderData::Size() -> unsigned int {
    static unsigned int size = 0u;
    if (size == 0u) {
        size = GlRuntimeHelper::GetUboAlignedSize(sizeof(Terrain::ShaderData));
    }
    return size;
}

Terrain::Terrain(std::vector<string> && diffuseMapFiles, string heightMap)
    : _diffuseMap(move(diffuseMapFiles), TextureUsage::DiffuseTextureArray)
    , _heightMap(heightMap, TextureUsage::HeightMap) {
    _shaderProgram.SetVertexShader("shader/terrain_v.shader");
    _shaderProgram.SetFragmentShader("shader/terrain_f.shader");
    _shaderProgram.SetTessellationControlShader("shader/terrain_tc.shader");
    _shaderProgram.SetTessellationEvaluationShader("shader/terrain_te.shader");
    //_diffuseMapFilename = "diffuse.dds";
    //_heightMapFilename = "heightMap.dds";
}

auto Terrain::Load(Float32 sightDistance) -> void {
    _sightDistance = sightDistance;
    _diffuseMap.Load();
    _heightMap.Load();
}

auto Terrain::AddSpecialTiles(std::vector<std::unique_ptr<Shape>> && shapes, vector<unique_ptr<Mesh<Vertex>>>&& meshes) -> void {
    _specialTileShapes = move(shapes);
    _specialTileMeshes = move(meshes);
    for (auto & shape : _specialTileShapes) {
        auto center = shape->GetAabb().GetCenter();
        _holeTiles.push_back(Vector2i{ static_cast<int>(floor(center(0) / _tileSize)), static_cast<int>(floor(center(1) / _tileSize)) });
    }
}

auto Terrain::GetHeight(Vector2f coord) const -> Float32 {
    auto heightMapCoord = static_cast<Vector2f>(coord / _tileSize - _heightMapOrigin);
    auto texel = _heightMap.GetBilinearFilteredTexel(heightMapCoord(0) / _heightMapSize(0), heightMapCoord(1) / _heightMapSize(1));
    return texel(0) * 30.0f - 0.6f; // todo: get rid of this dirty hard-coded expression
}

auto Terrain::CalculateTileCoordinate(Camera const* camera, vector<core::Vector3f> & tileCoords) -> void {
    auto startIndex = tileCoords.size();
    auto coverage = GetViewFrustumCoverage(camera);
    auto gridOrigin = Vector2i{ static_cast<int>(floor(coverage[0](0) / _tileSize)), static_cast<int>(floor(coverage[0](1) / _tileSize)) };
    auto gridSize = Vector2i{
        static_cast<int>(floor(coverage[1](0) / _tileSize)) + 1 - static_cast<int>(floor(coverage[0](0) / _tileSize)),
        static_cast<int>(floor(coverage[1](1) / _tileSize)) + 1 - static_cast<int>(floor(coverage[0](1) / _tileSize)) };
    for (auto i = 0; i < gridSize(0); ++i) {
        for (auto j = 0; j < gridSize(1); ++j) {
            auto coord = Vector3f{ (i + gridOrigin(0)) * _tileSize, (j + gridOrigin(1)) * _tileSize, 0 };
            tileCoords.push_back(coord);
        }
    }
    for (auto const& hole : _holeTiles) {
        auto delta = hole - gridOrigin;
        // hole tile out of scope
        if (delta(0) < 0 || delta(0) >= gridSize(0) || delta(1) < 0 || delta(1) >= gridSize(1)) {
            continue;
        }
        auto index = delta(0) * gridSize(1) + delta(1);
        tileCoords[index + startIndex](2) = -10;
    }
}

auto Terrain::GetTileCoordinate(Camera const* camera) -> std::vector<Vector3f> const& {
    _tileCoord.clear();
    CalculateTileCoordinate(camera, _tileCoord);
    return _tileCoord;
}

auto Terrain::GetTileCoordinateWithSpecialTiles(Camera const* camera) -> std::vector<Vector3f> const& {
    _tileCoord.clear();
    _tileCoord.insert(_tileCoord.begin(), _specialTileShapes.size(), core::Vector3f{ 0, 0, 0 });
    CalculateTileCoordinate(camera, _tileCoord);
    return _tileCoord;
}

auto Terrain::GetSpecialTiles() const -> std::vector<Mesh<Vertex> *> {
    auto ret = std::vector<Mesh<Vertex> *>{};
    for (auto const& mesh : _specialTileMeshes) {
        ret.push_back(mesh.get());
    }
    return ret;
}

auto Terrain::GetViewFrustumCoverage(Camera const * camera) const->std::array<Vector2f, 2>
{
    auto viewFrustumVertex = camera->GetViewFrustumVertex();
    auto lowerLeft = Vector2f{ std::numeric_limits<Float32>::max(), std::numeric_limits<Float32>::max() };
    auto upperRight = Vector2f{ std::numeric_limits<Float32>::lowest(), std::numeric_limits<Float32>::lowest() };
    for (auto const& p : viewFrustumVertex) {
        if (p(0) < lowerLeft(0)) {
            lowerLeft(0) = p(0);
        }
        if (p(0) > upperRight(0)) {
            upperRight(0) = p(0);
        }
        if (p(1) < lowerLeft(1)) {
            lowerLeft(1) = p(1);
        }
        if (p(1) > upperRight(1)) {
            upperRight(1) = p(1);
        }
    }
    return array<Vector2f, 2>{lowerLeft, upperRight};
}

}
