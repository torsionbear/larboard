#pragma once

#include "Matrix.h"
#include "Movable.h"

namespace core {

class Camera : public Movable {
public:
	struct ShaderData {
		Matrix4x4f viewTransform;
		Vector4f viewPosition;
	};
public:
	Camera();

public:
	auto SetPerspective(Float32 near, Float32 far, Vector2f lowerLeft, Vector2f upperRight) -> void;
	auto SetPerspective(Float32 near, Float32 far, Float32 fieldOfView, Float32 aspectRatio) -> void;
	auto GetProjectionTransform() const -> Matrix4x4f const&;

	auto GetShaderData() -> ShaderData;
	auto GetUboOffset() const -> int {
		return _uboOffset;
	}
	auto SetUboOffset(int offset) -> void {
		_uboOffset = offset;
	}
private:
	Matrix4x4f _projectionTransform;
	openglInt _uboOffset;
};

}