#pragma once

#include "Movable.h"

namespace core {

class SpotLight : public Movable{
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
        return _direction;
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
private:
	Vector4f _color;
	Vector4f _direction;
	Vector4f _attenuation;
	Float32 _beamWidth;
	Float32 _cutOffAngle;
};

}