#pragma once

#include "Movable.h"

namespace core {

class SpotLight : public Movable {
public:
    struct ShaderData {
        Vector4f color;
        Vector4f position;
        Vector4f direction;
        Vector4f attenuation;
        Float32 beamWidth;
        Float32 cutOffAngle;
        Float32 pad[2];
    };
public:
    auto SetColor(Vector4f color) -> void {
        _color = color;
    }
    auto GetColor() -> Vector4f {
        return _color;
    }
    auto SetDirection(Vector4f direction) -> void {
        _direction = direction;
    }
    auto GetDirection() -> Vector4f {
        return GetTransform() * _direction;
    }
    auto SetRadius(Float32 radius) -> void {
        _attenuation(3) = radius;
    }
    auto SetAttenuation(Vector3f attenuation) -> void {
        _attenuation(0) = attenuation(0);
        _attenuation(1) = attenuation(1);
        _attenuation(2) = attenuation(2);
    }
    auto SetAttenuation(Vector4f attenuation) -> void {
        _attenuation = attenuation;
    }
    auto GetAttenuation() -> Vector4f {
        return _attenuation;
    }
    auto SetBeamWidth(Float32 beamWidth) -> void {
        _beamWidth = beamWidth;
    }
    auto GetBeamWidth() -> Float32 {
        return _beamWidth;
    }
    auto SetCutOffAngle(Float32 cutOffAngle) -> void {
        _cutOffAngle = cutOffAngle;
    }
    auto GetCutOffAngle() -> Float32 {
        return _cutOffAngle;
    }
    auto GetShaderData() -> ShaderData {
        return ShaderData{
            _color,
            GetPosition(),
            GetTransform() * _direction,
            _attenuation,
            _beamWidth,
            _cutOffAngle,
            { 0.0f, 0.0f },
        };
    }
    auto GetRenderDataId() const -> unsigned int {
        return _renderDataId;
    }
    auto SetRenderDataId(unsigned int id) -> void {
        _renderDataId = id;
    }
    auto GetLightVolume() -> Mesh<VertexC3> * {
        if (_lightVolume == nullptr) {
            PopulateLightVolume();
        }
        return _lightVolume.get();
    }
private:
    auto PopulateLightVolume() -> void {
        auto vertexData = std::vector<VertexC3>{ 
            VertexC3{ Vector3f{ -0.382682f, 0.923880f, -1.000000f } },
            VertexC3{ Vector3f{ 0.000000f, 0.000000f, 0.000000f } },
            VertexC3{ Vector3f{ -0.195089f, 0.980786f, -1.000000f } },
            VertexC3{ Vector3f{ 0.000000f, 1.000000f, -1.000000f } },
            VertexC3{ Vector3f{ 0.195090f, 0.980785f, -1.000000f } },
            VertexC3{ Vector3f{ -0.555569f, 0.831470f, -1.000000f } },
            VertexC3{ Vector3f{ -0.707106f, 0.707108f, -1.000000f } },
            VertexC3{ Vector3f{ -0.831469f, 0.555571f, -1.000000f } },
            VertexC3{ Vector3f{ -0.923879f, 0.382684f, -1.000000f } },
            VertexC3{ Vector3f{ -0.980785f, 0.195091f, -1.000000f } },
            VertexC3{ Vector3f{ -1.000000f, 0.000001f, -1.000000f } },
            VertexC3{ Vector3f{ -0.980785f, -0.195089f, -1.000000f } },
            VertexC3{ Vector3f{ -0.923880f, -0.382683f, -1.000000f } },
            VertexC3{ Vector3f{ -0.831470f, -0.555570f, -1.000000f } },
            VertexC3{ Vector3f{ -0.707107f, -0.707106f, -1.000000f } },
            VertexC3{ Vector3f{ -0.555571f, -0.831469f, -1.000000f } },
            VertexC3{ Vector3f{ -0.382684f, -0.923879f, -1.000000f } },
            VertexC3{ Vector3f{ -0.195091f, -0.980785f, -1.000000f } },
            VertexC3{ Vector3f{ -0.000000f, -1.000000f, -1.000000f } },
            VertexC3{ Vector3f{ 0.195090f, -0.980785f, -1.000000f } },
            VertexC3{ Vector3f{ 0.382683f, -0.923880f, -1.000000f } },
            VertexC3{ Vector3f{ 0.555570f, -0.831470f, -1.000000f } },
            VertexC3{ Vector3f{ 0.707107f, -0.707107f, -1.000000f } },
            VertexC3{ Vector3f{ 0.831470f, -0.555570f, -1.000000f } },
            VertexC3{ Vector3f{ 0.923880f, -0.382683f, -1.000000f } },
            VertexC3{ Vector3f{ 0.980785f, -0.195090f, -1.000000f } },
            VertexC3{ Vector3f{ 1.000000f, 0.000000f, -1.000000f } },
            VertexC3{ Vector3f{ 0.980785f, 0.195090f, -1.000000f } },
            VertexC3{ Vector3f{ 0.923880f, 0.382683f, -1.000000f } },
            VertexC3{ Vector3f{ 0.831470f, 0.555570f, -1.000000f } },
            VertexC3{ Vector3f{ 0.707107f, 0.707107f, -1.000000f } },
            VertexC3{ Vector3f{ 0.555570f, 0.831470f, -1.000000f } },
            VertexC3{ Vector3f{ 0.382683f, 0.923880f, -1.000000f } },
        };
        auto distance = _attenuation(3);
        auto radius = _attenuation(3) * tan(_cutOffAngle);
        for (auto & vertex : vertexData) {
            vertex.coord(0) *= radius;
            vertex.coord(1) *= radius;
            vertex.coord(2) *= distance;
        }
        auto indexData = std::vector<unsigned int>{
            0, 1, 2, 3, 1, 4, 5, 1, 0, 6, 1, 5, 7, 1, 6, 8, 1, 7, 9, 1, 8, 10, 1, 9, 11, 1, 10, 12, 1, 11, 13, 1, 12, 14, 1, 13, 15, 1, 14, 16, 1, 15, 17, 1, 16, 18, 1, 17, 19, 1, 18, 20, 1, 19, 21, 1, 20, 22, 1, 21, 23, 1, 22, 24, 1, 23, 25, 1, 24, 26, 1, 25, 27, 1, 26, 28, 1, 27, 29, 1, 28, 30, 1, 29, 31, 1, 30, 32, 1, 31, 2, 1, 3, 4, 1, 32, 3, 4, 2, 4, 32, 31, 31, 30, 29, 29, 28, 27, 27, 26, 25, 25, 24, 23, 23, 22, 21, 21, 20, 19, 19, 18, 17, 17, 16, 15, 15, 14, 13, 13, 12, 11, 11, 10, 9, 9, 8, 7, 7, 6, 5, 5, 0, 2, 4, 31, 2, 31, 29, 27, 27, 25, 23, 23, 21, 19, 19, 17, 15, 15, 13, 11, 11, 9, 7, 7, 5, 2, 31, 27, 2, 27, 23, 19, 19, 15, 11, 11, 7, 2, 27, 19, 2, 19, 11, 2,
        };
        _lightVolume = std::make_unique<Mesh<VertexC3>>(std::move(vertexData), std::move(indexData));
    }
private:
    Vector4f _color = Vector4f{ 1.0f, 1.0f, 1.0f, 1.0f };
    Vector4f _direction = Vector4f{ 0.0f, 0.0f, -1.0f, 0.0f };
    Vector4f _attenuation = Vector4f{ 1.0f, 0.0f, 0.4f, 25.0f };
    Float32 _beamWidth = 0.2f;
    Float32 _cutOffAngle = 0.3f;
    unsigned int _renderDataId;
    std::unique_ptr<Mesh<VertexC3>> _lightVolume = nullptr;
};

}