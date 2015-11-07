#pragma once

#include <vector>
#include <string>
#include <array>
#include <memory>

#include "Matrix.h"
#include "CubeMap.h"
#include "ShaderProgram.h"

namespace core {

class SkyBox {
public:
    explicit SkyBox(std::array<std::string, 6> && filenames)
        : _cubeMap(std::make_unique<CubeMap>(move(filenames)))
        , _shaderProgram(std::make_unique<ShaderProgram>("shader/skybox.vert", "shader/skybox.frag"))
        , _vertexData{
        Vector3f{ -1, 1, -1 },
        Vector3f{ -1, -1, -1 },
        Vector3f{ 1, -1, -1 },
        Vector3f{ 1, 1, -1 },
        Vector3f{ -1, 1, 1 },
        Vector3f{ -1, -1, 1 },
        Vector3f{ 1, -1, 1 },
        Vector3f{ 1, 1, 1 } }
        , _indexData{0, 1, 2, 0, 2, 3, 0, 5, 1, 0, 4, 5, 3, 4, 0, 3, 7, 4, 1, 6, 2, 1, 5, 6, 2, 7, 3, 2, 6, 7, 4, 6, 5, 4, 7, 6} {
    }
    ~SkyBox();
public:
    auto PrepareForDraw() -> void;
    auto Draw() -> void;
private:
    std::unique_ptr<CubeMap> _cubeMap;
    std::unique_ptr<ShaderProgram> _shaderProgram;
    std::vector<Vector3f> _vertexData;
    std::vector<unsigned int> _indexData;
    openglUint _vao;
    openglUint _vbo;
    openglUint _veo;
};

}