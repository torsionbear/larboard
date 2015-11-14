#include "terrain.h"

#include <GL/glew.h>

using std::string;

namespace core {

Terrain::Terrain(Float32 tileSize, Vector2i mapOrigin, Vector2i mapSize, vector<string> && diffuseMapFiles, string heightMap)
    : _mapOrigin(mapOrigin)
    , _mapSize(mapSize)
    , _tileSize(tileSize)
    , _shaderProgram("shader/terrain.vert", "shader/terrain.frag")
    , _tileVertexData{ Vector2f{ 0, 0 }, Vector2f{ tileSize, 0 }, Vector2f{ tileSize, tileSize }, Vector2f{ 0, tileSize } } 
    , _indexData{ 0, 1, 3, 1, 2, 3 }
    , _diffuseMap(move(diffuseMapFiles), TextureUsage::DiffuseTextureArray)
    , _heightMap(heightMap, TextureUsage::HeightMap) {
}

auto Terrain::PrepareForDraw(Float32 sightDistance) -> void {
    _tileCountInSight = sightDistance / _tileSize;

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _tileVertexData.size() * sizeof(Vector2f), _tileVertexData.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &_veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _veo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indexData.size() * sizeof(unsigned int), _indexData.data(), GL_STATIC_DRAW);

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
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "mapOrigin"), 1, _mapOrigin.data());
    glUniform2iv(glGetUniformLocation(_shaderProgram.GetHandler(), "mapSize"), 1, _mapSize.data());
    auto error = glGetError();

    _diffuseMap.Use();
    _heightMap.Use();
    glBindVertexArray(_vao);

    glDrawElementsInstanced(GL_TRIANGLES, _indexData.size(), GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0), 4 *_tileCountInSight * _tileCountInSight);
    //glDrawElements(GL_TRIANGLES, _indexData.size(), GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0));
    error = glGetError();
}

}
