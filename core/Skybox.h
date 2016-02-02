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
        , _shaderProgram(std::make_unique<ShaderProgram>("shader/skybox_v.shader", "shader/skybox_f.shader")) {
    }
public:
    auto Load() -> void {
        _cubeMap->Load();
        _shaderProgram->Load();
    }
    auto GetShaderProgram() -> ShaderProgram * {
        return _shaderProgram.get();
    }
    auto GetShaderProgram() const -> ShaderProgram const * {
        return _shaderProgram.get();
    }
    auto GetCubeMap() -> CubeMap * {
        return _cubeMap.get();
    }
    auto GetCubeMap() const -> CubeMap const * {
        return _cubeMap.get();
    }
    auto GetVao() const -> openglUint {
        return _vao;
    }
    auto SetVao(openglUint vao) -> void {
        _vao = vao;
    }
private:
    std::unique_ptr<CubeMap> _cubeMap;
    std::unique_ptr<ShaderProgram> _shaderProgram;

    openglUint _vao;
};

}