#pragma once

#include "Vector.h"

namespace core {

struct Vertex {
	Vector3f coord;
	Vector3f normal;
	Vector2f texCoord;
};

}