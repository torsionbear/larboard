#pragma once

#include "Matrix.h"

namespace core {

class Material {
public:
    struct ShaderData {
    public:
        Vector4f _diffuseEmissive;
        Vector4f _specularShininess;
    public:
        static auto Size() -> unsigned int;
    };
public:
    auto SetDiffuse(Vector3f diffuse) -> void {
        _diffuse = diffuse;
    }
    auto SetEmissive(Vector3f emissive) -> void {
        _emissive = emissive;
    }
    auto SetSpecular(Vector3f specular) -> void {
        _specular = specular;
    }
    auto SetShininess(Float32 shininess) -> void {
        _shininess = shininess;
    }
    auto SetTransparency(Float32 transparency) -> void {
        _transparency = transparency;
    }
    auto GetDiffuse() const -> Vector3f {
        return _diffuse;
    }
    auto GetEmissive() const -> Vector3f {
        return _emissive;
    }
    auto GetSpecular() const -> Vector3f {
        return _specular;
    }
    auto GetShininess() const -> Float32 {
        return _shininess;
    }
    auto GetTransparency() const -> Float32 {
        return _transparency;
    }
    auto SetUbo(openglUint ubo) -> void {
        _ubo = ubo;
    }
    auto GetUbo() const -> openglUint {
        return _ubo;
    }
    auto GetUboOffset() const -> int {
        return _uboOffset;
    }
    auto SetUboOffset(int offset) -> void {
        _uboOffset = offset;
    }
    auto GetShaderData() const -> ShaderData {
        return ShaderData{
            Vector4f{_diffuse(0), _diffuse(1), _diffuse(2), 0.0f},
            Vector4f{_specular(0), _specular(1), _specular(2), _shininess },
        };
    }
private:
    ShaderData _shaderData;
    Vector3f _diffuse;
    Vector3f _specular;
    Vector3f _emissive;
    Float32 _shininess;
    Float32 _transparency;
    openglUint _ubo;
    openglInt _uboOffset;
public:
    bool _hasDiffuseMap = false;
    bool _hasNormalMap = false;
    bool _hasSpecularMap = false;
    bool _hasEmissiveMap = false;
public:
    unsigned int _renderDataId;
};

}