#include "terrain.h"

#include <GL/glew.h>

using std::string;

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
    _tileCountInSight = sightDistance / _tileSize;

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    auto tileVertexData = std::array<Vector2f, 4>{ Vector2f{ 0, 0 }, Vector2f{ _tileSize, 0 }, Vector2f{ _tileSize, _tileSize }, Vector2f{ 0, _tileSize } };
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector2f), tileVertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &_veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _veo);
    auto indexData = std::array<unsigned int, 6>{ 0, 1, 3, 1, 2, 3 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    _diffuseMap.Load();
    _heightMap.SendToCard();
    _shaderProgram.SendToCard();
    auto error = glGetError();
}

auto Terrain::Draw() -> void {
    _shaderProgram.Use();
    //uniforms
    glUniform1i(glGetUniformLocation(_shaderProgram.GetHandler(), "tileCountInSight"), _tileCountInSight);
    glUniform1i(glGetUniformLocation(_shaderProgram.GetHandler(), "tileSize"), _tileSize);
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "heightMapOrigin"), 1, _heightMapOrigin.data());
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "heightMapSize"), 1, _heightMapSize.data());
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "diffuseMapOrigin"), 1, _diffuseMapOrigin.data());
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "diffuseMapSize"), 1, _diffuseMapSize.data());
    auto error = glGetError();

    _diffuseMap.Use();
    _heightMap.Use();
    glBindVertexArray(_vao);
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElementsInstanced(GL_PATCHES, 6, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0), 4 *_tileCountInSight * _tileCountInSight);

    error = glGetError();
}

}
