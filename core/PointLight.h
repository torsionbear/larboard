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
private:
	Vector4f _color;
	Vector4f _attenuation;
};

}