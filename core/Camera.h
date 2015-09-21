#pragma once

#include "Matrix.h"
#include "Vector.h"
#include "Movable.h"

namespace core {

class Camera : public Movable {
public:
	Camera();

public:
	auto SetPerspective(Float32 near, Float32 far, Vector2f lowerLeft, Vector2f upperRight) -> void;
	auto SetPerspective(Float32 near, Float32 far, Float32 fieldOfView, Float32 aspectRatio) -> void;
	auto GetProjectionTransform() const -> Matrix4x4f const&;

private:
	Matrix4x4f _projectionTransform;
};

}