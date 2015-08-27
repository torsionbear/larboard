#include "Camera.h"

#include <cmath>

namespace core {

auto Camera::GetProjectionTransform() const -> Matrix4x4f const & {
	return _projectionTransform;
}

auto Camera::SetPerspective(Float32 near, Float32 far, Vector2f lowerLeft, Vector2f upperRight) -> void {

	// third column has been negate to transform right-hand world space to left-hand screen space.
	_projectionTransform = Matrix4x4f{
		2 * near / (upperRight.x - lowerLeft.x), 0, (upperRight.x + lowerLeft.x) / (upperRight.x - lowerLeft.x), 0,
		0, 2 * near / (upperRight.y - lowerLeft.y), (upperRight.y + lowerLeft.y) / (upperRight.y - lowerLeft.y), 0,
		0, 0, -(far + near) / (far - near), -2 * far*near / (far - near),
		0, 0, -1, 0
	};
}

auto Camera::SetPerspective(Float32 near, Float32 far, Float32 aspectRatio, Float32 fieldOfView) -> void {
	auto halfWidth = near;
	auto halfHeight = near;
	auto & majorAxis = aspectRatio > 1 ? halfWidth : halfHeight;
	auto & minorAxis = aspectRatio > 1 ? halfHeight : halfWidth;

	// according to x3d standard:
	// where the smaller of display width or display height determines which angle equals the fieldOfView 
	// (the larger angle is computed using the relationship described above). 
	// The larger angle shall not exceed ¦Ð and may force the smaller angle to be less than fieldOfView 
	// in order to sustain the aspect ratio.
	// here we keep fieldOfView less than 5/6¦Ð.
	auto maxFieldOfView = pi * 5 / 6;
	auto tangent = aspectRatio * fieldOfView >= maxFieldOfView ? tan(maxFieldOfView / 2) : tan(fieldOfView / 2);
	majorAxis *= tangent;
	minorAxis *= tangent / aspectRatio;
	
	SetPerspective(near, far, { -halfWidth, -halfHeight }, { halfWidth, halfHeight });
}

}
