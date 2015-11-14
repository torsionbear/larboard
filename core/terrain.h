#pragma once

#include <array>

#include "Texture.h"
#include "TextureArray.h"
#include "Camera.h"
#include "ShaderProgram.h"

namespace core {

class Terrain {
public:
    Terrain(Float32 tileSize, Vector2i mapOrigin, Vector2i mapSize, std::vector<std::string> && diffuseMapFiles, std::string heightMap);
public:
    auto PrepareForDraw(Float32 sightDistance) -> void;
    auto Draw()  -> void;
private:
    std::array<Vector2f, 4> _tileVertexData;
    std::array<unsigned int, 6> _indexData;
    Vector2i _mapOrigin;
    Vector2i _mapSize;  // width and height in tile count
    Float32 _tileSize;
    int _tileCountInSight = 100u;

    TextureArray _diffuseMap;
    Texture _heightMap;
    ShaderProgram _shaderProgram;
    openglUint _vao;
    openglUint _vbo;
    openglUint _veo;
};

}