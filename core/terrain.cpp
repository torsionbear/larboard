#include "terrain.h"

#include <GL/glew.h>
#include <array>

#include "ResourceManager.h"

using std::string;
using std::array;
using std::unique_ptr;

namespace core {

Terrain::Terrain(vector<string> && diffuseMapFiles, string heightMap)
    : _diffuseMap(move(diffuseMapFiles), TextureUsage::DiffuseTextureArray)
    , _heightMap(heightMap, TextureUsage::HeightMap) {
    _shaderProgram.SetVertexShader("shader/terrain_v.shader");
    _shaderProgram.SetFragmentShader("shader/terrain_f.shader");
    _shaderProgram.SetTessellationControlShader("shader/terrain_tc.shader");
    _shaderProgram.SetTessellationEvaluationShader("shader/terrain_te.shader");
}

auto Terrain::PrepareForDraw(Float32 sightDistance) -> void {
    _sightDistance = sightDistance;
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    auto tileVertexData = std::array<Vector2f, 4>{ Vector2f{ 0, 0 }, Vector2f{ _tileSize, 0 }, Vector2f{ _tileSize, _tileSize }, Vector2f{ 0, _tileSize } };
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector2f), tileVertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &_veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _veo);
    auto indexData = std::array<unsigned int, 6>{ 0, 1, 3, 1, 2, 3 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &_vio);
    glBindBuffer(GL_ARRAY_BUFFER, _vio);
    auto tileCountUpperBound = static_cast<int>(ceil(_sightDistance / _tileSize) * 2);
    tileCountUpperBound *= tileCountUpperBound;
    glBufferData(GL_ARRAY_BUFFER, tileCountUpperBound * sizeof(Vector2i), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(1, 2, GL_INT, sizeof(Vector2i), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    _diffuseMap.Load();
    _heightMap.SendToCard();
    _shaderProgram.SendToCard();

    // load special tiles
    LoadSpecialTiles();
    auto error = glGetError();
}

auto Terrain::Draw(Camera const* camera) -> void {
    auto coverage = GetViewFrustumCoverage(camera);
    auto gridOrigin = Vector2i{ static_cast<int>(floor(coverage[0](0) / _tileSize)), static_cast<int>(floor(coverage[0](1) / _tileSize)) };
    auto gridSize = Vector2i{ static_cast<int>(floor(coverage[1](0) / _tileSize)) + 1, static_cast<int>(floor(coverage[1](1) / _tileSize)) + 1 } - gridOrigin;
    auto tileCoord = vector<Vector2i>{};
    for (auto i = 0; i < gridSize(0); ++i) {
        for (auto j = 0; j < gridSize(1); ++j) {
            auto coord = Vector2i{ i, j };
            tileCoord.push_back(gridOrigin + coord);
        }
    }
    for (auto const& hole : _holeTiles) {
        auto delta = hole - gridOrigin;
        auto index = delta(0) * gridSize(1) + delta(1);
        // quick delete
        tileCoord[index] = tileCoord.back();
        tileCoord.pop_back();
    }
    glBindBuffer(GL_ARRAY_BUFFER, _vio);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vector2i) * tileCoord.size(), tileCoord.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    _shaderProgram.Use();
    //uniforms
    glUniform1f(glGetUniformLocation(_shaderProgram.GetHandler(), "tileSize"), _tileSize);
    glUniform1f(glGetUniformLocation(_shaderProgram.GetHandler(), "sightDistance"), _sightDistance);
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "gridOrigin"), 1, gridOrigin.data());
    glUniform1i(glGetUniformLocation(_shaderProgram.GetHandler(), "gridWidth"), gridSize(0));

    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "heightMapOrigin"), 1, _heightMapOrigin.data());
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "heightMapSize"), 1, _heightMapSize.data());
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "diffuseMapOrigin"), 1, _diffuseMapOrigin.data());
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "diffuseMapSize"), 1, _diffuseMapSize.data());
    auto error = glGetError();

    _diffuseMap.Use();
    _heightMap.Use();
    glBindVertexArray(_vao);
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElementsInstanced(GL_PATCHES, 6, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0), tileCoord.size());
    glBindVertexArray(0);

    // draw special tiles
    DrawSpecialTiles();
    error = glGetError();
}

auto Terrain::AddSpecialTiles(std::vector<std::unique_ptr<Shape>> && shapes, vector<unique_ptr<Mesh>>&& meshes) -> void {
    _specialTileShapes = move(shapes);
    _specialTileMeshes = move(meshes);
    for (auto & shape : _specialTileShapes) {
        auto center = shape->GetAabb().GetCenter();
        _holeTiles.push_back(Vector2i{static_cast<int>(floor(center(0) / _tileSize)), static_cast<int>(floor(center(1) / _tileSize)) });
    }
}

auto Terrain::GetViewFrustumCoverage(Camera const * camera)->std::array<Vector2f, 2>
{
    auto near = camera->GetNearPlane();
    auto far = camera->GetFarPlane();
    auto halfWidth = camera->GetHalfWidth();
    auto halfHeight = camera->GetHalfHeight();
    auto farHalfWidth = halfWidth * far / near;
    auto farHalfHeight = halfHeight * far / near;
    auto transform = camera->GetTransform();
    auto viewFrustumVertex = array<Point4f, 8>{
        transform * Point4f{ -halfWidth, -halfHeight, -near, 1 },
            transform * Point4f{ -halfWidth, halfHeight, -near, 1 },
            transform * Point4f{ halfWidth, -halfHeight, -near, 1 },
            transform * Point4f{ halfWidth, halfHeight, -near, 1 },
            transform * Point4f{ -farHalfWidth, -farHalfHeight, -far, 1 },
            transform * Point4f{ -farHalfWidth, farHalfHeight, -far, 1 },
            transform * Point4f{ farHalfWidth, -farHalfHeight, -far, 1 },
            transform * Point4f{ farHalfWidth, farHalfHeight, -far, 1 },
    };
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

auto Terrain::LoadSpecialTiles() -> void {
    glGenVertexArrays(1, &_specialTilesVao);
    glBindVertexArray(_specialTilesVao);

    glGenBuffers(1, &_specialTilesVbo);
    glBindBuffer(GL_ARRAY_BUFFER, _specialTilesVbo);

    glGenBuffers(1, &_specialTilesVeo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _specialTilesVeo);

    auto vertexData = vector<Vector2f>();
    auto indexData = vector<unsigned int>();
    for (auto const& mesh : _specialTileMeshes) {
        mesh->SetVertexArrayObject(_specialTilesVao);
        mesh->SetBaseVertex(vertexData.size());
        mesh->SetIndexOffset(indexData.size() * sizeof(unsigned int));
        for (auto const& vertex : mesh->GetVertex()) {
            vertexData.push_back(Vector2f{ vertex.coord(0), vertex.coord(1) });
        }
        indexData.insert(indexData.end(), mesh->GetIndex().cbegin(), mesh->GetIndex().cend());
    }
    auto tileCoordData = vector<Vector2i>(vertexData.size(), Vector2i{ 0, 0 });
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector2f) + tileCoordData.size() * sizeof(Vector2i), vertexData.data(), GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(Vector2f), vertexData.data());
    glBufferSubData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vector2f), tileCoordData.size() * sizeof(Vector2i), tileCoordData.data());

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(1, 2, GL_INT, sizeof(Vector2i), (GLvoid*)(vertexData.size() * sizeof(Vector2f)));
    glEnableVertexAttribArray(1);

    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(Vector3f)));
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(Vector3f)));
    //glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    auto error = glGetError();
}

auto Terrain::DrawSpecialTiles() -> void {
    for (auto & mesh : _specialTileMeshes) {
        mesh->Draw(Mesh::patches);
    }
}

}
