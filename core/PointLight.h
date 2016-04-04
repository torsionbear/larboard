#pragma once

#include "Matrix.h"
#include "Movable.h"

namespace core {

class PointLight : public Movable {
public:
    struct ShaderData {
        Vector4f color;
        Vector4f position;
        Vector4f attenuation;
    };
public:
    auto static GetLightVolume() -> Mesh<VertexC3> * {
        static auto pointLightVolume = Mesh<VertexC3>(
            std::vector<Float32>{ 0.000000f, 0.000000f, -1.000000f, 0.425323f, -0.309011f, -0.850654f, -0.162456f, -0.499995f, -0.850654f, 0.723607f, -0.525725f, -0.447220f, 0.850648f, 0.000000f, -0.525736f, -0.525730f, 0.000000f, -0.850652f, -0.162456f, 0.499995f, -0.850654f, 0.425323f, 0.309011f, -0.850654f, 0.951058f, -0.309013f, 0.000000f, -0.276388f, -0.850649f, -0.447220f, 0.262869f, -0.809012f, -0.525738f, 0.000000f, -1.000000f, 0.000000f, -0.894426f, 0.000000f, -0.447216f, -0.688189f, -0.499997f, -0.525736f, -0.951058f, -0.309013f, 0.000000f, -0.276388f, 0.850649f, -0.447220f, -0.688189f, 0.499997f, -0.525736f, -0.587786f, 0.809017f, 0.000000f, 0.723607f, 0.525725f, -0.447220f, 0.262869f, 0.809012f, -0.525738f, 0.587786f, 0.809017f, 0.000000f, 0.587786f, -0.809017f, 0.000000f, -0.587786f, -0.809017f, 0.000000f, -0.951058f, 0.309013f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.951058f, 0.309013f, 0.000000f, 0.276388f, -0.850649f, 0.447220f, 0.688189f, -0.499997f, 0.525736f, 0.162456f, -0.499995f, 0.850654f, -0.723607f, -0.525725f, 0.447220f, -0.262869f, -0.809012f, 0.525738f, -0.425323f, -0.309011f, 0.850654f, -0.723607f, 0.525725f, 0.447220f, -0.850648f, 0.000000f, 0.525736f, -0.425323f, 0.309011f, 0.850654f, 0.276388f, 0.850649f, 0.447220f, -0.262869f, 0.809012f, 0.525738f, 0.162456f, 0.499995f, 0.850654f, 0.894426f, 0.000000f, 0.447216f, 0.688189f, 0.499997f, 0.525736f, 0.525730f, 0.000000f, 0.850652f, 0.000000f, 0.000000f, 1.000000f, },
            std::vector<unsigned int>{ 0, 1, 2, 3, 1, 4, 0, 2, 5, 0, 5, 6, 0, 6, 7, 3, 4, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 3, 8, 21, 9, 11, 22, 12, 14, 23, 15, 17, 24, 18, 20, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 2, 10, 9, 2, 1, 10, 1, 3, 10, 4, 7, 18, 4, 1, 7, 1, 0, 7, 5, 13, 12, 5, 2, 13, 2, 9, 13, 6, 16, 15, 6, 5, 16, 5, 12, 16, 7, 19, 18, 7, 6, 19, 6, 15, 19, 8, 25, 38, 8, 4, 25, 4, 18, 25, 11, 21, 26, 11, 10, 21, 10, 3, 21, 14, 22, 29, 14, 13, 22, 13, 9, 22, 17, 23, 32, 17, 16, 23, 16, 12, 23, 20, 24, 35, 20, 19, 24, 19, 15, 24, 21, 27, 26, 21, 8, 27, 8, 38, 27, 22, 30, 29, 22, 11, 30, 11, 26, 30, 23, 33, 32, 23, 14, 33, 14, 29, 33, 24, 36, 35, 24, 17, 36, 17, 32, 36, 25, 39, 38, 25, 20, 39, 20, 35, 39, 28, 40, 41, 28, 27, 40, 27, 38, 40, 31, 28, 41, 31, 30, 28, 30, 26, 28, 34, 31, 41, 34, 33, 31, 33, 29, 31, 37, 34, 41, 37, 36, 34, 36, 32, 34, 40, 37, 41, 40, 39, 37, 39, 35, 37, }
        );
        return &pointLightVolume;
    }
public:
    auto SetColor(Vector4f value) -> void {
        _color = value;
    }
    auto GetColor() -> Vector4f {
        return _color;
    }
    auto SetRadius(Float32 value) -> void {
        _attenuation(3) = value;
    }
    auto SetAttenuation(Vector3f value) -> void {
        _attenuation(0) = value(0);
        _attenuation(1) = value(1);
        _attenuation(2) = value(2);
    }
    auto SetAttenuation(Vector4f value) -> void {
        _attenuation = value;
    }
    auto GetAttenuation() -> Vector4f {
        return _attenuation;
    }
    auto GetShaderData() const -> ShaderData {
        return ShaderData{
            _color,
            GetPosition(),
            _attenuation,
        };
    }
    auto GetRenderDataId() const -> unsigned int {
        return _renderDataId;
    }
    auto SetRenderDataId(unsigned int id) -> void {
        _renderDataId = id;
    }
private:
    Vector4f _color = Vector4f{ 1.0f, 1.0f, 1.0f, 1.0f };
    Vector4f _attenuation = Vector4f{ 1.0f, 0.0f, 0.4f, 25.0f };

    unsigned int _renderDataId;
};

}